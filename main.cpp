#include <Windows.h>
#include <memory>
#include <format>

// DirectX12
#include <d3d12.h>
#include <dxgi1_6.h>
#include <cassert>
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")



#include "Engine/WIndow/WinApp.h"

using namespace std;

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{

	unique_ptr<WinApp> winApp = make_unique<WinApp>();
	winApp->Initialize();

#pragma region DirectX12 Initialize
#pragma region DirectX Graphics Infrastructure
	// ファクトリ生成
	IDXGIFactory7 *dxgiFactory = nullptr;
	// エラーコードの取得
	HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));
	// SUCCEEDEDマクロで判定
	assert(SUCCEEDED(hr));

	// 使用アダプタ用
	IDXGIAdapter4 *useAdapter = nullptr;
	// 性能順で選別
	for (UINT i = 0;
		dxgiFactory->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&useAdapter)) != DXGI_ERROR_NOT_FOUND;
		++i)
	{
		// 情報習得
		DXGI_ADAPTER_DESC3 adapterDesc{};
		hr = useAdapter->GetDesc3(&adapterDesc);
		assert(SUCCEEDED(hr));

		// ソフトウェア以外であれば採用
		if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE))
		{
			// 使用するアダプタをログに出力
			Log(ConvertToString(std::format(L"Use Adapter:{}\n", adapterDesc.Description)));
			break;
		}
		useAdapter = nullptr;
	}

	assert(useAdapter != nullptr);

	ID3D12Device *device = nullptr;
	// 機能レベルとログ出力用文字列
	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_12_2,
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0
	};
	const char *featureLevelStrings[] = {
		"12.2", "12.1", "12.0"
	};
	// 性能順に生成テスト
	for (size_t i = 0; i < _countof(featureLevels); ++i)
	{
		// 生成
		hr = D3D12CreateDevice(useAdapter, featureLevels[i], IID_PPV_ARGS(&device));
		// 指定した機能レベルで生成できたか確認
		if (SUCCEEDED(hr))
		{
			// 生成できていればループ終了
			Log(std::format("FeatureLevel : {}\n", featureLevelStrings[i]));
			break;
		}
	}
	// デバイスが生成できているか確認
	assert(device != nullptr);
	Log("Complete Create D3D12Device\n");
#pragma endregion 
#pragma region Command Initialize
	// コマンドキュー生成
	ID3D12CommandQueue *commandQueue = nullptr;
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
	hr = device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue));
	// コマンドキュー生成確認
	assert(SUCCEEDED(hr));
	Log("CreateCommandQueue\n");
	// コマンドアロケータ生成
	ID3D12CommandAllocator *commandAllocator = nullptr;
	hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
	// コマンドアロケータ生成確認
	assert(SUCCEEDED(hr));
	Log("CreateCommandAllocator\n");
	// コマンドリスト生成
	ID3D12GraphicsCommandList *commandList = nullptr;
	hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator, nullptr, IID_PPV_ARGS(&commandList));
	// コマンドリスト生成確認
	assert(SUCCEEDED(hr));
	Log("CreateCommandList\n");
#pragma endregion
#pragma region SwapChain Create
	// スワップチェーン生成
	IDXGISwapChain4 *swapChain = nullptr;
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	swapChainDesc.Width = WinApp::kWindoWidth;
	swapChainDesc.Height = WinApp::kWindoHeight;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.SampleDesc.Count = 1;								//マルチサンプルしない
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;	// 描画ターゲットとして利用する
	swapChainDesc.BufferCount = 2;									// ダブルバッファ
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;		// 画面に移したら内容破棄
	// コマンドキュー、オウィンドウハンドル、設定渡して生成
	hr = dxgiFactory->CreateSwapChainForHwnd(commandQueue, winApp->GetHWND(), &swapChainDesc, nullptr, nullptr, reinterpret_cast<IDXGISwapChain1 **>(&swapChain));
	// スワップチェイン生成確認
	assert(SUCCEEDED(hr));
	Log("CreateSwapChain\n");
#pragma endregion
#pragma region DescriptorHeap Initialize
	ID3D12DescriptorHeap *rtvDescriptorHeap = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC rtvDescriptorHeapDesc{};
	// RTV用で用意
	rtvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	// ダブルバッファ用に2つ
	rtvDescriptorHeapDesc.NumDescriptors = 2;
	hr = device->CreateDescriptorHeap(&rtvDescriptorHeapDesc, IID_PPV_ARGS(&rtvDescriptorHeap));
	// ディスクリプタヒープ生成確認
	assert(SUCCEEDED(hr));
	Log("CreateRTVDecritorHeap\n");
#pragma endregion
#pragma region GetResourcesFromSwapChain
	ID3D12Resource *swapChainResources[2] = { nullptr };
	hr = swapChain->GetBuffer(0, IID_PPV_ARGS(&swapChainResources[0]));
	assert(SUCCEEDED(hr));
	hr = swapChain->GetBuffer(1, IID_PPV_ARGS(&swapChainResources[1]));
	assert(SUCCEEDED(hr));
	Log("GetResourcesFromSwapChain\n");
#pragma endregion
#pragma region CreateRenderTargetView
	// RTVSettings
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;		// 出力結果をSRGBにして書き込み
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;	// 2Dテクスチャとして書き込み
	// RTVを2つ作成するためディスクリプタも2つ用意
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2];
	// ディスクリプタの先頭に1つ目を作成
	rtvHandles[0] = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	device->CreateRenderTargetView(swapChainResources[0], &rtvDesc, rtvHandles[0]);
	// 先頭からディスクリプタのサイズ分移動した場所のハンドルを取得
	rtvHandles[1].ptr = rtvHandles[0].ptr + device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	// 2つ目を作成
	device->CreateRenderTargetView(swapChainResources[1], &rtvDesc, rtvHandles[1]);

#pragma endregion
#pragma endregion

	while (!winApp->PoccesMessage())
	{

	}
	swapChainResources[1]->Release();
	swapChainResources[0]->Release();
	rtvDescriptorHeap->Release();
	swapChain->Release();
	commandList->Release();
	commandAllocator->Release();
	commandQueue->Release();
	device->Release();
	useAdapter->Release();
	dxgiFactory->Release();
	return 0;
}