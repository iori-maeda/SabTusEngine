#include "TextureManager.h"

#include <cassert>

#include "StringUtility.h"
#include "Logger.h"
#include "DirectXCommon.h"
#include "DxDevice.h"

std::string TextureManager::defaultDirectoryPath = "Resources/Textures/";

uint32_t TextureManager::textureIndex = 3;	// 0は現状ImGuiが利用する

TextureManager &TextureManager::GetInstace()
{
	static TextureManager instance;
	return instance;
}

void TextureManager::Initialize(DirectXCommon *dxCommon)
{
	assert(dxCommon != nullptr);

	dxCommon_ = dxCommon;

	textures_.reserve(DirectXCommon::kMaxDescriptorCountSRV - textureIndex);
}
void TextureManager::Finalize()
{
	textures_.clear();
}

TextureDataCPU TextureManager::Load(const std::string &fileName)
{
	return CreateTextureData(defaultDirectoryPath, fileName);
}

TextureDataCPU TextureManager::Load(const std::string &directoryPath, const std::string &fileName)
{
	return CreateTextureData(directoryPath, fileName);
}

D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetSRVDescriptorGPUHandle(const std::string &key)
{
	// 読み込み済みなら要素を返して早期リターン
	auto textureData = std::find_if(textures_.begin(), textures_.end(),
		[&](const TextureData &t) { return t.fileName == key; });
	if (textureData != textures_.end())
	{

		return textureData->gpuData.srvGpuHandle;
	}

	Logger::Log("Texture not found : " + key + "\n");
	return D3D12_GPU_DESCRIPTOR_HANDLE{};
}

TextureDataCPU TextureManager::CreateTextureData(const std::string directoryPath, const std::string &filePath)
{
	TextureData newTextureData{};
	//TextureDataCPU textureDataCPU{};
	//TextureDataGPU textureDataGPU{};

	// 読み込み済みなら要素を返して早期リターン
	auto textureData = std::find_if(textures_.begin(), textures_.end(),
		[&](const TextureData &t) { return t.fileName == filePath; });
	if (textureData != textures_.end())
	{

		return textureData->cpuData;
	}

	// ファイル名の保存
	newTextureData.directoryPath = directoryPath;
	newTextureData.fileName = filePath;
	newTextureData.cpuData.fileName = filePath;

	DirectX::ScratchImage mipImage = LoadTexture(newTextureData.directoryPath + newTextureData.fileName);

	newTextureData.cpuData.metaData = mipImage.GetMetadata();

	newTextureData.gpuData.resource = dxCommon_->CreateTextureResource(mipImage.GetMetadata());

	ComPtr<ID3D12Resource> intermediateResource = dxCommon_->UploadTextureData(newTextureData.gpuData.resource, mipImage);

	// 中間リソース転送待ち
	dxCommon_->WaitForGPU();

	// ハンドルの取得 
	newTextureData.gpuData.srvCpuHandle = dxCommon_->GetSRVDescriptorCPUHandle(textureIndex);
	newTextureData.gpuData.srvGpuHandle = dxCommon_->GetSRVDescriptorGPUHandle(textureIndex);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = newTextureData.cpuData.metaData.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = static_cast<UINT>(newTextureData.cpuData.metaData.mipLevels);

	// SRV生成
	dxCommon_->GetDxDevice()->GetDevice()->CreateShaderResourceView(newTextureData.gpuData.resource.Get(), &srvDesc, newTextureData.gpuData.srvCpuHandle);

	// 追加
	textures_.emplace_back(newTextureData);
	textureIndex++;

	// CPU用のデータのみ返却
	return newTextureData.cpuData;
}

DirectX::ScratchImage TextureManager::LoadTexture(const std::string &filePath)
{
	// TextureFileプログラム用に読み込む
	DirectX::ScratchImage image{};
	std::wstring filePathW = StringUtility::ConvertToWString(filePath);
	HRESULT hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
	assert(SUCCEEDED(hr));
	Logger::Log("Texture Load\n");

	if (image.GetMetadata().mipLevels <= 1)
	{
		return image;
	}
	DirectX::ScratchImage mipImage{};
	hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImage);
	Logger::Log("MipMap Create\n");
	return mipImage;
}