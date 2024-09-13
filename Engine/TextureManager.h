#pragma once

#include <string>
#include <d3d12.h>
#include "../externals/DirectXTex/DirectXTex.h"

#include "ComPtr.h"

class DxDevice;
class DxCommand;
class DxFence;

struct TextureData
{
	DirectX::ScratchImage mipImages;
	DirectX::TexMetadata metaData;
	ComPtr<ID3D12Resource> resource = nullptr;
	ComPtr<ID3D12Resource> intermediateResource = nullptr;
	D3D12_CPU_DESCRIPTOR_HANDLE texSrvHandleCPU;
	D3D12_GPU_DESCRIPTOR_HANDLE texSrvHandleGPU;
};

class TextureManager
{
public:

	void Initialize(DxDevice*, DxCommand*, ID3D12DescriptorHeap*);
	TextureData LoadTexrureData(const std::string &);
	bool WaitToUploadTextureDataForGPU(DxFence*);
	void InitializeDescriptorHandles(D3D12_CPU_DESCRIPTOR_HANDLE&, D3D12_GPU_DESCRIPTOR_HANDLE&);
	void CreateSRVByTexuter(const TextureData&);

	// ゲッターより
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(uint32_t, uint32_t);
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(uint32_t, uint32_t);

private:

	DirectX::ScratchImage LoadTexture(const std::string&);
	ComPtr<ID3D12Resource> CreateTextureResource(const ComPtr<ID3D12Device>&, const DirectX::TexMetadata&);
	[[nodiscard]]
	ComPtr<ID3D12Resource> UploadTextureData(const ComPtr<ID3D12Resource>&, const DirectX::ScratchImage&, const ComPtr<ID3D12Device>&, const ComPtr<ID3D12GraphicsCommandList>&);


private:

	const std::string kDirectoryPath = "Resources/Textures/";

	ID3D12Device4* device_ = nullptr;
	DxCommand* command_ = nullptr;
	ID3D12DescriptorHeap* heap_ = nullptr;


	uint32_t indexCount_ = 1;
};