#pragma once
#include <string>
#include <memory>
#include <vector>

#include "../externals/DirectXTex/DirectXTex.h"
#include "../externals/DirectXTex/d3dx12.h"

#include "ComPtr.h"

class DirectXCommon;

struct TextureDataCPU
{
	std::string fileName;
	DirectX::TexMetadata metaData;
};

class TextureManager
{
public:
	static std::string defaultDirectoryPath;

	// シングルトンパターンで作成
	static TextureManager& GetInstace();

	void Initialize(DirectXCommon* renderContext);
	void Finalize();

	/// <summary>
	/// デフォルト設定のディレクトリから画像読み込み
	/// </summary>
	/// <param name="fileName">画像名.拡張子</param>
	/// <returns></returns>
	TextureDataCPU Load(const std::string& fileName);
	/// <summary>
	/// ディレクトリ指定で画像読み込み
	/// </summary>
	/// <param name="directoryPath">保存先ディレクトリパス</param>
	/// <param name="fileName">画像名.拡張子</param>
	/// <returns></returns>
	TextureDataCPU Load(const std::string& directoryPath, const std::string& fileName);

public:

	D3D12_GPU_DESCRIPTOR_HANDLE GetSRVDescriptorGPUHandle(const std::string& key);
	
private:
	TextureManager() = default;
	TextureManager(const TextureManager&) = delete;
	TextureManager& operator=(const TextureManager&) = delete;


private:

	TextureDataCPU CreateTextureData(const std::string directoryPath, const std::string& filePath);
	DirectX::ScratchImage LoadTexture(const std::string& filePath);


private:
	DirectXCommon* dxCommon_ = nullptr;


	struct TextureDataGPU {
		ComPtr<ID3D12Resource> resource = nullptr;
		D3D12_CPU_DESCRIPTOR_HANDLE srvCpuHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE srvGpuHandle;
	};

	struct TextureData
	{
		std::string directoryPath;
		std::string fileName;
		TextureDataCPU cpuData{};
		TextureDataGPU gpuData{};
	};

	static uint32_t textureIndex;

	std::vector<TextureData> textures_;
};

