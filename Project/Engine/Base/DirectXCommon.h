#pragma once
#include <cstdint>
#include <string>
#include <memory>
#include <d3d12.h>

#include "../externals/DirectXTex/DirectXTex.h"
#include "../externals/DirectXTex/d3dx12.h"

#include "ComPtr.h"

class WinApp;
class DxDevice;
class DxCommand;
class DxSwapChain;
class DxFence;

class DirectXCommon
{
public:
	static const uint32_t kMaxDescriptorCountSRV = 256;

public:

	DirectXCommon() = default;
	~DirectXCommon();
	void Initialize(const WinApp& winApp);
	void BeginRendering();
	void EndRendering();

	void WaitForGPU();

	ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptor, bool shaderVisible);
	ComPtr<ID3D12Resource> CreateDepthStencilTextureResource(int32_t width, int32_t height);
	ComPtr<ID3D12Resource> CreateBufferResource(size_t sizeInBytes) const;
	ComPtr<ID3D12Resource> CreateTextureResource(const DirectX::TexMetadata& metaData);
	[[nodiscard]] ComPtr<ID3D12Resource> UploadTextureData(const ComPtr<ID3D12Resource>& texture, const DirectX::ScratchImage& mipImage);


public:

	DxDevice* GetDxDevice() const { return device.get(); }
	DxCommand* GetCommand() const { return command.get(); }
	DxSwapChain* GetSwapChain() const { return swapChain.get(); }

	ID3D12DescriptorHeap* GetRTVDescriptorHeap() const { return rtvDescriptorHeap.Get(); }
	ID3D12DescriptorHeap* GetSRVDescriptorHeap() const { return srvDescriptorHeap.Get(); }
	ID3D12DescriptorHeap* GetDSVDescriptorHeap() const { return dsvDescriptorHeap.Get(); }

	D3D12_RENDER_TARGET_VIEW_DESC GetRTVDesc() const { return rtvDesc; }

	D3D12_CPU_DESCRIPTOR_HANDLE GetRTVDescriptorCPUHandle(uint32_t index);
	D3D12_GPU_DESCRIPTOR_HANDLE GetRTVDescriptorGPUHandle(uint32_t index);

	D3D12_CPU_DESCRIPTOR_HANDLE GetSRVDescriptorCPUHandle(uint32_t index);
	D3D12_GPU_DESCRIPTOR_HANDLE GetSRVDescriptorGPUHandle(uint32_t index);

	D3D12_CPU_DESCRIPTOR_HANDLE GetDSVDescriptorCPUHandle(uint32_t index);
	D3D12_GPU_DESCRIPTOR_HANDLE GetDSVDescriptorGPUHandle(uint32_t index);

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(const ComPtr<ID3D12DescriptorHeap>& descriptorHeap, uint32_t descriptorSize, uint32_t index);
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(const ComPtr<ID3D12DescriptorHeap>& descriptorHeap, uint32_t descriptorSize, uint32_t index);

	void SetClearColor(float r, float g, float b, float a)
	{
		clearColor[0] = r;
		clearColor[1] = g;
		clearColor[2] = b;
		clearColor[3] = a;
	};


private:
	void CreateDescriptorHeaps();
	void CreateRenderTaegetrView();
	void CreateDepthStencilView();
	void CreateViewPortAndSiccorRect();

private:

	std::unique_ptr<DxDevice> device = nullptr;
	std::unique_ptr<DxCommand> command = nullptr;
	std::unique_ptr<DxSwapChain> swapChain = nullptr;
	std::unique_ptr<DxFence> fence = nullptr;

	UINT backBufferIndex = 0;

	// ディスクリプターヒープ
	ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap = nullptr;
	ComPtr<ID3D12DescriptorHeap> srvDescriptorHeap = nullptr;
	ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap = nullptr;

	uint32_t rtvDescriptorSize = 0;
	uint32_t srvDescriptorSize = 0;
	uint32_t dsvDescriptorSize = 0;

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	// RTVを2つ作成するためディスクリプタも2つ用意
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2]{};

	ComPtr<ID3D12Resource> depthStencilResource = nullptr;
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle{};

	D3D12_VIEWPORT viewport{};
	// scissor
	D3D12_RECT scissorRect{};

	float clearColor[4] = { 0.1f, 0.25f, 0.5f, 1.0f };
};

