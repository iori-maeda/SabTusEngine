#include "TextureManager.h"

#include <cassert>

#include "../externals/DirectXTex/d3dx12.h"

#include "Logger.h"
#include "StringUtility.h"
#include "DirectX12Objects/DirectX12ObjectsFunction.h"
#include "DirectX12Objects/DxDevice.h"
#include "DirectX12Objects/DxCommand.h"
#include "DirectX12Objects/DxFence.h"

std::map<std::string, TextureData> TextureManager::texMap_;

/// <summary>
/// 借り物の借り受け
/// </summary>
/// <param name="device"></param>
/// <param name="command"></param>
/// <param name="fence"></param>
/// <param name="heap"></param>
void TextureManager::Initialize(DxDevice *device, DxCommand *command, DxFence *fence, ID3D12DescriptorHeap *heap)
{
	device_ = device->GetDevice();
	command_ = command;
	fence_ = fence;
	heap_ = heap;
}

/// <summary>
/// 画像読込と登録
/// </summary>
/// <param name="fileName">画像名</param>
/// <param name="registrationKey">登録するキー、省略する場合は画像名の拡張し無しで登録する</param>
/// <returns></returns>
TextureData TextureManager::LoadTexrureData(const std::string &fileName, const std::string& registrationKey)
{
	// KEYをファイル名から決定
	std::string key = registrationKey == "" ? fileName.substr(0, fileName.find_last_of(".")) : registrationKey;

	// 画像読込
	DirectX::ScratchImage mipImages = LoadTexture(kDirectoryPath + fileName);

	// 格納用データ
	TextureData result{};
	result.metaData = mipImages.GetMetadata();
	result.resource = CreateTextureResource(device_, result.metaData);
	result.intermediateResource = UploadTextureData(result.resource, mipImages, device_, command_->GetCommandList());

	// 転送待ち
	WaitToUploadTextureDataForGPU();
	// ハンドル作成
	InitializeDescriptorHandles(result.texSrvHandleCPU, result.texSrvHandleGPU);
	// SRVに新しく領域を確保
	CreateSRVByTexuter(result);

	// マップへ挿入
	texMap_.emplace(key, result);
	return result;
}

/// <summary>
/// 画像データの取得
/// </summary>
/// <param name="key">登録してあるキー</param>
/// <returns></returns>
TextureData& TextureManager::GetTextureData(const std::string& key)
{
	auto itr = texMap_.find(key) ;
	assert(itr != texMap_.end());
	return itr->second;
}

bool TextureManager::WaitToUploadTextureDataForGPU()
{
	// CommandList Close & Kick
	command_->Close();
	// Fence Wait
	fence_->IncrementFenceValue();
	command_->GetCommandQueue()->Signal(fence_->GetFence(), fence_->GetFenceValue());
	fence_->WaitSignalToGPU();
	// commandList Reset
	command_->Reset();

	return true;
}

void TextureManager::InitializeDescriptorHandles(D3D12_CPU_DESCRIPTOR_HANDLE &CPUHandle, D3D12_GPU_DESCRIPTOR_HANDLE &GPUHandle)
{
	CPUHandle = heap_->GetCPUDescriptorHandleForHeapStart();
	CPUHandle.ptr += static_cast<SIZE_T>(device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * indexCount_);

	GPUHandle = heap_->GetGPUDescriptorHandleForHeapStart();
	GPUHandle.ptr += static_cast<SIZE_T>(device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * indexCount_);

	++indexCount_;
}

void TextureManager::CreateSRVByTexuter(const TextureData &data)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = data.metaData.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = static_cast<UINT>(data.metaData.mipLevels);

	device_->CreateShaderResourceView(data.resource.Get(), &srvDesc, data.texSrvHandleCPU);
}

D3D12_CPU_DESCRIPTOR_HANDLE TextureManager::GetCPUDescriptorHandle(uint32_t discriptorSize, uint32_t index)
{
	D3D12_CPU_DESCRIPTOR_HANDLE result = heap_->GetCPUDescriptorHandleForHeapStart();
	result.ptr += static_cast<SIZE_T>(discriptorSize * index);
	return result;
}

D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetGPUDescriptorHandle(uint32_t discriptorSize, uint32_t index)
{
	D3D12_GPU_DESCRIPTOR_HANDLE result = heap_->GetGPUDescriptorHandleForHeapStart();;
	result.ptr += static_cast<SIZE_T>(discriptorSize * index);
	return result;
}

/// <summary>
/// ミップマップ付きデータの取得
/// </summary>
/// <param name="filePath"></param>
/// <returns></returns>
DirectX::ScratchImage TextureManager::LoadTexture(const std::string &filePath)
{
	// TextureFileえおプログラム用に読み込む
	DirectX::ScratchImage image{};
	std::wstring filePathW = StringUtility::ConvertToWString(filePath);
	HRESULT hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
	assert(SUCCEEDED(hr));
	Logger::Log("Texture Load\n");

	DirectX::ScratchImage mipImage{};
	hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImage);
	Logger::Log("MipMap Create\n");
	return mipImage;
}

/// <summary>
/// テクスチャリソースの作成
/// </summary>
/// <param name="device">作成してくれるデバイス</param>
/// <param name="metaData">作成元データ</param>
/// <returns></returns>
ComPtr<ID3D12Resource> TextureManager::CreateTextureResource(const ComPtr<ID3D12Device> &device, const DirectX::TexMetadata &metaData)
{
	// metaDataからResourceの設定を取得
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = static_cast<UINT>(metaData.width);
	resourceDesc.Height = static_cast<UINT>(metaData.height);
	resourceDesc.MipLevels = static_cast<UINT16>(metaData.mipLevels);
	resourceDesc.DepthOrArraySize = static_cast<UINT16>(metaData.arraySize);
	resourceDesc.Format = metaData.format;
	resourceDesc.SampleDesc.Count = 1; // サンプリングカウント1固定
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION(metaData.dimension);

	// Heap設定
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	//heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;	// writeBackポリシーでcpuアクセス許可
	//heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;				// プロセッサの近くに配置

	// Resource生成
	ComPtr<ID3D12Resource> resource = nullptr;
	HRESULT hr = device->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_COPY_DEST, // データ転送設定
		nullptr,
		IID_PPV_ARGS(&resource)
	);
	assert(SUCCEEDED(hr));
	Logger::Log("TextureResource Created\n");
	return resource;
}

/// <summary>
/// 中間リソースの作成とアップロード
/// </summary>
/// <param name="texture">中間リソースを作成するリソース</param>
/// <param name="mipImage">元データ</param>
/// <param name="device">作成してくれるデバイス</param>
/// <param name="commandList">アップロードコマンド積込みと実行用</param>
/// <returns>中間リソース転送完了まで破棄しないこと</returns>
ComPtr<ID3D12Resource> TextureManager::UploadTextureData(const ComPtr<ID3D12Resource> &texture, const DirectX::ScratchImage &mipImage, const ComPtr<ID3D12Device> &device, const ComPtr<ID3D12GraphicsCommandList> &commandList)
{
	// 中間リソースの作成までを別関数にわかるべきか？
	std::vector<D3D12_SUBRESOURCE_DATA> subresources;
	DirectX::PrepareUpload(device.Get(), mipImage.GetImages(), mipImage.GetImageCount(), mipImage.GetMetadata(), subresources);
	uint64_t intermediateSize = GetRequiredIntermediateSize(texture.Get(), 0, static_cast<UINT>(subresources.size()));
	ComPtr<ID3D12Resource> intermediateResource = Dx12ObjFuncs::CreataeBufferResource(device, intermediateSize);

	// どうやったらこの関数の使用をやめれる？
	UpdateSubresources(commandList.Get(), texture.Get(), intermediateResource.Get(), 0, 0, static_cast<UINT>(subresources.size()), subresources.data());

	// プロシージャかなんかで裏で待機させたいよね
	// 転送後、コピーからリードへ変更
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = texture.Get();
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;
	commandList->ResourceBarrier(1, &barrier);
	Logger::Log("MipMap Upload To Texture\n");
	return intermediateResource;
}