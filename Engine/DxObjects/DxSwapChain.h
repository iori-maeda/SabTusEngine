#pragma once

#include <array>
#include <d3d12.h>
#include <dxgi1_6.h>

#include "ComPtr.h"

class WinApp;
class DxDevice;
class DxCommand;

class DxSwapChain
{
public:
	static const UINT kBufferCount = 2;

	void Initialize(const WinApp& winApp, DxDevice*, DxCommand*);
	void Present(UINT, UINT);


	ID3D12Resource* GetSwapChainResourceByIndex(int);
	D3D12_RESOURCE_BARRIER* GetBarrier(D3D12_RESOURCE_STATES, D3D12_RESOURCE_STATES);
	UINT GetBackBufferIndex();

private:

	ComPtr<IDXGISwapChain4> swapChain_ = nullptr;
	std::array<ComPtr<ID3D12Resource>, kBufferCount> swapChainResources_ = { nullptr };
	D3D12_RESOURCE_BARRIER barrier_{};
	UINT backBufferIndex_ = 0;
};

