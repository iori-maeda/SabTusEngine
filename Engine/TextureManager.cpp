#include "TextureManager.h"

#include <cassert>

#include "StringUtility.h"
#include "Logger.h"
#include "DirectXCommon.h"
#include "DirectX12Objects//DxDevice.h"

std::string TextureManager::defaultDirectoryPath = "Resources/Textures/";

uint32_t TextureManager::textureIndex = 1;	// 0は現状ImGuiが利用する

TextureManager& TextureManager::GetInstace()
{
	static TextureManager instance;
	return instance;
}

void TextureManager::Initialize(DirectXCommon* dxCommon)
{
	assert(dxCommon != nullptr);

	renderContext_ = dxCommon;
}
void TextureManager::Finalize()
{
	textures_.clear();
}

TextureDataCPU TextureManager::Load(const std::string& fileName)
{
	TextureDataCPU textureDataCPU{};
	TextureDataGPU textureDataGPU{};

	// 読み込み済みなら要素を返して早期リターン
	auto textureData = textures_.find(fileName);
	if (textureData != textures_.end())
	{
		textureDataCPU = textureData->second.first;
		textureDataGPU = textureData->second.second;
		return textureDataCPU;
	}

	// ファイル名の保存
	textureDataCPU.fileName = fileName;
	textureDataGPU.fileName = fileName;

	DirectX::ScratchImage mipImage = LoadTexture(defaultDirectoryPath + fileName);

	textureDataCPU.metaData = mipImage.GetMetadata();

	textureDataGPU.resource = renderContext_->CreateTextureResource(mipImage.GetMetadata());

	ComPtr<ID3D12Resource> intermediateResource = renderContext_->UploadTextureData(textureDataGPU.resource, mipImage);

	// 中間リソース転送待ち
	renderContext_->WaitForGPU();

	// ハンドルの取得 
	textureDataGPU.srvCpuHandle = renderContext_->GetSRVDescriptorCPUHandle(textureIndex);
	textureDataGPU.srvGpuHandle = renderContext_->GetSRVDescriptorGPUHandle(textureIndex);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = textureDataCPU.metaData.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = static_cast<UINT>(textureDataCPU.metaData.mipLevels);

	// SRV生成
	renderContext_->GetDxDevice()->GetDevice()->CreateShaderResourceView(textureDataGPU.resource.Get(), &srvDesc, textureDataGPU.srvCpuHandle);

	// 追加
	textures_.emplace(fileName, std::make_pair(textureDataCPU, textureDataGPU));
	textureIndex++;

	// CPU用のデータのみ返却
	return textureDataCPU;
}

TextureDataCPU TextureManager::Load(const std::string& directoryPath, const std::string& fileName)
{
	TextureDataCPU textureDataCPU{};
	TextureDataGPU textureDataGPU{};

	// 読み込み済みなら要素を返して早期リターン
	auto textureData = textures_.find(fileName);
	if (textureData != textures_.end())
	{
		textureDataCPU = textureData->second.first;
		textureDataGPU = textureData->second.second;
		return textureDataCPU;
	}

	// ファイル名の保存
	textureDataCPU.fileName = fileName;
	textureDataGPU.fileName = fileName;

	DirectX::ScratchImage mipImage = LoadTexture(directoryPath + fileName);

	textureDataCPU.metaData = mipImage.GetMetadata();

	textureDataGPU.resource = renderContext_->CreateTextureResource(mipImage.GetMetadata());

	ComPtr<ID3D12Resource> intermediateResource = renderContext_->UploadTextureData(textureDataGPU.resource, mipImage);

	// 中間リソース転送待ち
	renderContext_->WaitForGPU();


	// ハンドルの取得 
	textureDataGPU.srvCpuHandle = renderContext_->GetSRVDescriptorCPUHandle(textureIndex);
	textureDataGPU.srvGpuHandle = renderContext_->GetSRVDescriptorGPUHandle(textureIndex);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = textureDataCPU.metaData.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = static_cast<UINT>(textureDataCPU.metaData.mipLevels);

	renderContext_->GetDxDevice()->GetDevice()->CreateShaderResourceView(textureDataGPU.resource.Get(), &srvDesc, textureDataGPU.srvCpuHandle);

	// 追加
	textures_.emplace(fileName, std::make_pair(textureDataCPU, textureDataGPU));
	textureIndex++;

	// CPU用のデータのみ返却
	return textureDataCPU;
}

D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetSRVDescriptorGPUHandle(const std::string& key)
{
	auto textureData = textures_.find(key);
	if (textureData != textures_.end())
	{
		return textureData->second.second.srvGpuHandle;
	}

	Logger::Log("Texture not found : " + key + "\n");
	return D3D12_GPU_DESCRIPTOR_HANDLE{};
}

DirectX::ScratchImage TextureManager::LoadTexture(const std::string& filePath)
{
	// TextureFileプログラム用に読み込む
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