#include "DxRenderContext.h"

#include <assert.h>

#include "WIndow/WinApp.h"
#include "DirectX12Objects/DxDevice.h"
#include "DirectX12Objects/DxCommand.h"
#include "DirectX12Objects/DxSwapChain.h"
#include "DirectX12Objects/DxFence.h"

#include"Logger.h"
#include "../externals/DirectXTex/d3dx12.h"

using namespace std;

void DxRenderContext::Initialize(const WinApp& winApp)
{
	device = make_unique<DxDevice>();
	device->Initialize();

	command = make_unique<DxCommand>();
	command->Initialize(device.get());

	swapChain = make_unique<DxSwapChain>();
	swapChain->Initialize(winApp, device.get(), command.get());

	backBufferIndex = swapChain->GetBackBufferIndex();

	fence = make_unique<DxFence>();
	fence->Initialize(device.get());

	CreateDescriptorHeaps();
	CreateRenderTaegetrView();
	CreateDepthStencilView();
	CreateViewPortAndSiccorRect();


}

void DxRenderContext::BeginRendering()
{
	command->GetCommandList()->RSSetViewports(1, &viewport);
	command->GetCommandList()->RSSetScissorRects(1, &scissorRect);

	// Windor Drawing Step
	// State Present -> Render
	// Transition Barrier Set
	command->GetCommandList()->ResourceBarrier(1, swapChain->GetBarrier(D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
	backBufferIndex = swapChain->GetBackBufferIndex();

	// 描画先のRTｖをバックバッファのインデックスをもとに設定
	command->GetCommandList()->OMSetRenderTargets(1, &rtvHandles[backBufferIndex], false, nullptr);
	// 画面全体をクリア
	float clearColor[] = { 0.1f, 0.25f, 0.5f, 1.0f };
	command->GetCommandList()->ClearRenderTargetView(rtvHandles[backBufferIndex], clearColor, 0, nullptr);

	command->GetCommandList()->OMSetRenderTargets(1, &rtvHandles[backBufferIndex], false, &dsvHandle);
	command->GetCommandList()->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	ID3D12DescriptorHeap* descriptorHeaps[] = { srvDescriptorHeap.Get() };
	command->GetCommandList()->SetDescriptorHeaps(1, descriptorHeaps);
	command->GetCommandList()->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void DxRenderContext::EndRendering()
{
	// Windor Drawing Step
	// State Render -> Present
	// Transition Barrier Set
	command->GetCommandList()->ResourceBarrier(1, swapChain->GetBarrier(D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
	// CommandList Close & Kick
	command->CommandListCloseAndExecute();
	// GPUとOSに画面の交換を依頼を通知
	swapChain->Present(1, 0);
	// Fence Wait
	fence->IncrementFenceValue();
	command->GetCommandQueue()->Signal(fence->GetFence(), fence->GetFenceValue());
	fence->WaitSignalToGPU();
	// commandList Reset
	command->CommandListReset();
}

void DxRenderContext::WaitForGPU()
{
	// CommandList Close & Kick
	command->CommandListCloseAndExecute();
	// Fence Wait
	fence->IncrementFenceValue();
	command->GetCommandQueue()->Signal(fence->GetFence(), fence->GetFenceValue());
	fence->WaitSignalToGPU();
	// commandList Reset
	command->CommandListReset();
}

D3D12_CPU_DESCRIPTOR_HANDLE DxRenderContext::GetRTVDescriptorCPUHandle(uint32_t index)
{
	return GetCPUDescriptorHandle(rtvDescriptorHeap, rtvDescriptorSize, index);
}

D3D12_GPU_DESCRIPTOR_HANDLE DxRenderContext::GetRTVDescriptorGPUHandle(uint32_t index)
{
	return GetGPUDescriptorHandle(rtvDescriptorHeap, rtvDescriptorSize, index);
}

D3D12_CPU_DESCRIPTOR_HANDLE DxRenderContext::GetSRVDescriptorCPUHandle(uint32_t index)
{
	return GetCPUDescriptorHandle(srvDescriptorHeap, srvDescriptorSize, index);
}

D3D12_GPU_DESCRIPTOR_HANDLE DxRenderContext::GetSRVDescriptorGPUHandle(uint32_t index)
{
	return GetGPUDescriptorHandle(srvDescriptorHeap, srvDescriptorSize, index);
}

D3D12_CPU_DESCRIPTOR_HANDLE DxRenderContext::GetDSVDescriptorCPUHandle(uint32_t index)
{
	return GetCPUDescriptorHandle(dsvDescriptorHeap, dsvDescriptorSize, index);
}

D3D12_GPU_DESCRIPTOR_HANDLE DxRenderContext::GetDSVDescriptorGPUHandle(uint32_t index)
{
	return GetGPUDescriptorHandle(dsvDescriptorHeap, dsvDescriptorSize, index);
}

D3D12_CPU_DESCRIPTOR_HANDLE DxRenderContext::GetCPUDescriptorHandle(const ComPtr<ID3D12DescriptorHeap>& descriptorHeap, uint32_t descriptorSize, uint32_t index)
{
	D3D12_CPU_DESCRIPTOR_HANDLE result = descriptorHeap->GetCPUDescriptorHandleForHeapStart();;
	result.ptr += static_cast<SIZE_T>(descriptorSize * index);
	return result;
}

D3D12_GPU_DESCRIPTOR_HANDLE DxRenderContext::GetGPUDescriptorHandle(const ComPtr<ID3D12DescriptorHeap>& descriptorHeap, uint32_t descriptorSize, uint32_t index)
{
	D3D12_GPU_DESCRIPTOR_HANDLE result = descriptorHeap->GetGPUDescriptorHandleForHeapStart();;
	result.ptr += static_cast<UINT64>(descriptorSize * index);
	return result;
}

void DxRenderContext::CreateDescriptorHeaps()
{
	rtvDescriptorHeap = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 2, false);
	srvDescriptorHeap = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 128, true);
	dsvDescriptorHeap = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);

	rtvDescriptorSize = device->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	srvDescriptorSize = device->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	dsvDescriptorSize = device->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
}

void DxRenderContext::CreateRenderTaegetrView()
{
	// RTVSettings
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;		// 出力結果をSRGBにして書き込み
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;	// 2Dテクスチャとして書き込み

	//// ディスクリプタの先頭に1つ目を作成
	//rtvHandles[0] = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	//device->GetDevice()->CreateRenderTargetView(swapChain->GetSwapChainResourceByIndex(0), &rtvDesc, rtvHandles[0]);
	//// 先頭からディスクリプタのサイズ分移動した場所のハンドルを取得
	//rtvHandles[1].ptr = rtvHandles[0].ptr + device->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	//// 2つ目を作成
	//device->GetDevice()->CreateRenderTargetView(swapChain->GetSwapChainResourceByIndex(1), &rtvDesc, rtvHandles[1]);

	for (int i = 0; i < 2; i++)
	{
		rtvHandles[i] = GetRTVDescriptorCPUHandle(i);
		device->GetDevice()->CreateRenderTargetView(swapChain->GetSwapChainResourceByIndex(i), &rtvDesc, rtvHandles[i]);
	}
}

void DxRenderContext::CreateDepthStencilView()
{
	depthStencilResource = CreateDepthStencilTextureResource(WinApp::kWindoWidth, WinApp::kWindoHeight);

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;	// 基本Resouceにあわせる
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	// DSVHeapの先頭に作成
	device->GetDevice()->CreateDepthStencilView(depthStencilResource.Get(), &dsvDesc, dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	dsvHandle = dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
}

void DxRenderContext::CreateViewPortAndSiccorRect()
{
	// 画面全体に表示
	viewport.Width = static_cast<FLOAT>(WinApp::kWindoWidth);
	viewport.Height = static_cast<FLOAT>(WinApp::kWindoHeight);
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;


	// 基本的にはviewportと同じ構成で
	scissorRect.left = 0;
	scissorRect.right = WinApp::kWindoWidth;
	scissorRect.top = 0;
	scissorRect.bottom = WinApp::kWindoHeight;
}

ComPtr<ID3D12DescriptorHeap> DxRenderContext::CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible)
{
	ComPtr<ID3D12DescriptorHeap> descriptorHeap = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
	descriptorHeapDesc.Type = heapType;
	descriptorHeapDesc.NumDescriptors = numDescriptors;
	descriptorHeapDesc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	HRESULT hr = device->GetDevice()->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&descriptorHeap));
	// ディスクリプタヒープ生成確認
	assert(SUCCEEDED(hr));
	Logger::Log("CreateDecritorHeap\n");
	return descriptorHeap;
}

ComPtr<ID3D12Resource> DxRenderContext::CreateDepthStencilTextureResource(int32_t width, int32_t height)
{
	// metaDataからResourceの設定を取得
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = width;
	resourceDesc.Height = height;
	resourceDesc.MipLevels = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	resourceDesc.SampleDesc.Count = 1; // サンプリングカウント1固定
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	// Heap設定
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

	// 深度値のクリア設定
	CD3DX12_CLEAR_VALUE depthClearValue{};
	depthClearValue.DepthStencil.Depth = 1.0f;				// 最大値でクリア
	depthClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;	// Resourceとあわせる

	// Resource生成
	ComPtr<ID3D12Resource> resource = nullptr;
	HRESULT hr = device->GetDevice()->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE, // データ転送設定
		&depthClearValue,
		IID_PPV_ARGS(&resource)
	);
	assert(SUCCEEDED(hr));
	Logger::Log("TextureResource Created\n");
	return resource;
}

ComPtr<ID3D12Resource> DxRenderContext::CreataeBufferResource(size_t sizeInBytes)
{
	D3D12_HEAP_PROPERTIES uploadHeapPoperties{};
	uploadHeapPoperties.Type = D3D12_HEAP_TYPE_UPLOAD;
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Width = sizeInBytes;
	/// バッファのここはテンプレ
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	/// ここまでテンプレ
	ComPtr<ID3D12Resource> resource = nullptr;
	HRESULT hr = device->GetDevice()->CreateCommittedResource(&uploadHeapPoperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&resource));
	assert(SUCCEEDED(hr));
	Logger::Log("Created Resource\n");
	return resource;
}
