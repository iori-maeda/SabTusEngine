#include "DxSwapChain.h"

#include <cassert>
#include "../Logger.h"
#include "../Window/WinApp.h"
#include "DxDevice.h"
#include "DxCommand.h"

void DxSwapChain::Initialize(WinApp* winApp, DxDevice* device, DxCommand* command)
{
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	swapChainDesc.Width = WinApp::kWindoWidth;
	swapChainDesc.Height = WinApp::kWindoHeight;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.SampleDesc.Count = 1;								//マルチサンプルしない
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;	// 描画ターゲットとして利用する
	swapChainDesc.BufferCount = 2;									// ダブルバッファ
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;		// 画面に移したら内容破棄
	// コマンドキュー、オウィンドウハンドル、設定渡して生成
	HRESULT hr = device->GetFactory()->CreateSwapChainForHwnd(command->GetCommandQueue(), winApp->GetHWND(), &swapChainDesc, nullptr, nullptr, reinterpret_cast<IDXGISwapChain1**>(swapChain_.GetAddressOf()));
	// スワップチェイン生成確認
	assert(SUCCEEDED(hr));
	Logger::Log("CreateSwapChain\n");

	// Get Resources From SwapChain
	hr = swapChain_->GetBuffer(0, IID_PPV_ARGS(&swapChainResources_[0]));
	assert(SUCCEEDED(hr));
	hr = swapChain_->GetBuffer(1, IID_PPV_ARGS(&swapChainResources_[1]));
	assert(SUCCEEDED(hr));
	Logger::Log("GetResourcesFromSwapChain\n");

	backBufferIndex_ = swapChain_->GetCurrentBackBufferIndex();

	// Type Trainsition
	barrier_.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	// Flag NONE
	barrier_.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
}

void DxSwapChain::Present(UINT SyncIntervalFrame, UINT isTearingAllowed)
{
	swapChain_->Present(SyncIntervalFrame, isTearingAllowed);
}

ID3D12Resource* DxSwapChain::GetSwapChainResourceByIndex(int index)
{
	return swapChainResources_[index].Get();
}

D3D12_RESOURCE_BARRIER* DxSwapChain::GetBarrier(D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after)
{
	backBufferIndex_ = swapChain_->GetCurrentBackBufferIndex();
	// Barrier To Target
	barrier_.Transition.pResource = swapChainResources_[backBufferIndex_].Get();
	// Before Resource State
	barrier_.Transition.StateBefore = before;
	// After Resource State
	barrier_.Transition.StateAfter = after;

	return &barrier_;
}

UINT DxSwapChain::GetBackBufferIndex()
{
	return backBufferIndex_;
}
