#include <Windows.h>
#include <memory>
#include <format>
#include <cassert>
#include <vector>

// DirectX12
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
// Debug
#include <dxgidebug.h>
#pragma comment(lib, "dxguid.lib")
// ShaderComplier
#include <dxcapi.h>
#pragma comment(lib, "dxcompiler.lib")
// ImGui
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"

#include "externals/DirectXTex/DirectXTex.h"
#include "externals/DirectXTex/d3dx12.h"

#include "Engine/WIndow/WinApp.h"
// Math
#include "Engine/Math/Vector2.h"
#include "Engine/Math/Vector4.h"
#include "Engine/Math/Matrix4x4.h"

using namespace std;
using namespace Microsoft::WRL;

// 後々フォルダとh用意する
struct  Transform
{
	Vector3 scale{};
	Vector3 rotate{};
	Vector3 translate{};
};

struct VertexData
{
	Vector4 position{};
	Vector2 uv{};
};

struct D3DResourceLeakChecker
{
	~D3DResourceLeakChecker()
	{
		// Resources Leak Check
		ComPtr<IDXGIDebug1> debug;
		if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debug))))
		{
			debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
			debug->ReportLiveObjects(DXGI_DEBUG_APP, DXGI_DEBUG_RLO_ALL);
			debug->ReportLiveObjects(DXGI_DEBUG_D3D12, DXGI_DEBUG_RLO_ALL);
		}
	}
};

#pragma region functoins
ComPtr<IDxcBlob> CompileShader(const std::wstring&, const wchar_t*, const ComPtr<IDxcUtils>&, const ComPtr<IDxcCompiler3>&, const ComPtr<IDxcIncludeHandler>&);
ComPtr<ID3D12Resource> CreataeBufferResource(const ComPtr<ID3D12Device>&, size_t);
ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(const ComPtr<ID3D12Device>&, D3D12_DESCRIPTOR_HEAP_TYPE, UINT, bool);

// Texture用
DirectX::ScratchImage LoadTexture(const std::string&);
ComPtr<ID3D12Resource> CreateTextureResource(const ComPtr<ID3D12Device>&, const DirectX::TexMetadata&);
ComPtr<ID3D12Resource> UploadTextureData(const ComPtr<ID3D12Resource>&, const DirectX::ScratchImage&, const ComPtr<ID3D12Device>&, const ComPtr<ID3D12GraphicsCommandList>&);
// DepthStencil用
ComPtr<ID3D12Resource> CreateDepthStencilTextureResource(const ComPtr<ID3D12Device>&, int32_t, int32_t);
#pragma endregion

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	D3DResourceLeakChecker leckChecker;
	// Texture読み込みのためCOMを初期化
	CoInitializeEx(0, COINIT_MULTITHREADED);
	unique_ptr<WinApp> winApp = make_unique<WinApp>();
	winApp->Initialize();

#ifdef _DEBUG
	ComPtr<ID3D12Debug1> debugController = nullptr;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
		// デバッグレイヤー有効化
		debugController->EnableDebugLayer();
		// GPU側でもチェックを開始
		debugController->SetEnableGPUBasedValidation(TRUE);
	}
#endif
#pragma region DirectX Graphics Infrastructure & Create Device
	// ファクトリ生成
	ComPtr<IDXGIFactory7> dxgiFactory = nullptr;
	// エラーコードの取得
	HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));
	// SUCCEEDEDマクロで判定
	assert(SUCCEEDED(hr));

	// 使用アダプタ用
	ComPtr<IDXGIAdapter4> useAdapter = nullptr;
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

	ComPtr<ID3D12Device> device = nullptr;
	// 機能レベルとログ出力用文字列
	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_12_2,
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0
	};
	const char* featureLevelStrings[] = {
		"12.2", "12.1", "12.0"
	};
	// 性能順に生成テスト
	for (size_t i = 0; i < _countof(featureLevels); ++i)
	{
		// 生成
		hr = D3D12CreateDevice(useAdapter.Get(), featureLevels[i], IID_PPV_ARGS(&device));
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
#ifdef _DEBUG
	ComPtr<ID3D12InfoQueue> infoQueue = nullptr;
	if (SUCCEEDED(device->QueryInterface(IID_PPV_ARGS(&infoQueue)))) {
		// 重要度 高　エラー
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
		// 重要度 中　エラー
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
		// 重要度 低　警告
		//infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);

		// 抑制するメッセージのID
		D3D12_MESSAGE_ID denyIds[] = {
			// Windows11でのDXGIデバッグレイヤーとDX12デバッグレイヤーの相互作用バグによるメッセージ
			// https://stackoverflow.com/questions/69805245/directx-12-apllication-is-crashing-in-windows-11
			D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE
		};
		// 抑制するレベル
		D3D12_MESSAGE_SEVERITY severties[] = { D3D12_MESSAGE_SEVERITY_INFO };
		D3D12_INFO_QUEUE_FILTER filter{};
		filter.DenyList.NumIDs = _countof(denyIds);
		filter.DenyList.pIDList = denyIds;
		filter.DenyList.NumSeverities = _countof(severties);
		filter.DenyList.pSeverityList = severties;
		// 指定したメッセージの表示を抑制
		infoQueue->PushStorageFilter(&filter);
	}
#endif
#pragma region Command Initialize
	// コマンドキュー生成
	ComPtr<ID3D12CommandQueue> commandQueue = nullptr;
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
	hr = device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue));
	// コマンドキュー生成確認
	assert(SUCCEEDED(hr));
	Log("CreateCommandQueue\n");
	// コマンドアロケータ生成
	ComPtr<ID3D12CommandAllocator> commandAllocator = nullptr;
	hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
	// コマンドアロケータ生成確認
	assert(SUCCEEDED(hr));
	Log("CreateCommandAllocator\n");
	// コマンドリスト生成
	ComPtr<ID3D12GraphicsCommandList> commandList = nullptr;
	hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList));
	// コマンドリスト生成確認
	assert(SUCCEEDED(hr));
	Log("CreateCommandList\n");
#pragma endregion

#pragma region SwapChain Create
	// スワップチェーン生成
	ComPtr<IDXGISwapChain4> swapChain = nullptr;
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	swapChainDesc.Width = WinApp::kWindoWidth;
	swapChainDesc.Height = WinApp::kWindoHeight;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.SampleDesc.Count = 1;								//マルチサンプルしない
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;	// 描画ターゲットとして利用する
	swapChainDesc.BufferCount = 2;									// ダブルバッファ
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;		// 画面に移したら内容破棄
	// コマンドキュー、オウィンドウハンドル、設定渡して生成
	hr = dxgiFactory->CreateSwapChainForHwnd(commandQueue.Get(), winApp->GetHWND(), &swapChainDesc, nullptr, nullptr, reinterpret_cast<IDXGISwapChain1**>(swapChain.GetAddressOf()));
	// スワップチェイン生成確認
	assert(SUCCEEDED(hr));
	Log("CreateSwapChain\n");
#pragma endregion

#pragma region DescriptorHeap Initialize
	ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap = CreateDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 2, false);
	ComPtr<ID3D12DescriptorHeap> srvDescriptorHeap = CreateDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 128, true);
	ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap = CreateDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);
#pragma endregion

#pragma region Get Resources From SwapChain
	ComPtr<ID3D12Resource> swapChainResources[2] = { nullptr };
	hr = swapChain->GetBuffer(0, IID_PPV_ARGS(&swapChainResources[0]));
	assert(SUCCEEDED(hr));
	hr = swapChain->GetBuffer(1, IID_PPV_ARGS(&swapChainResources[1]));
	assert(SUCCEEDED(hr));
	Log("GetResourcesFromSwapChain\n");
#pragma endregion

#pragma region RenderTargetView Create
	// RTVSettings
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;		// 出力結果をSRGBにして書き込み
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;	// 2Dテクスチャとして書き込み
	D3D12_CPU_DESCRIPTOR_HANDLE rtvStartHandle = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	// RTVを2つ作成するためディスクリプタも2つ用意
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2];
	// ディスクリプタの先頭に1つ目を作成
	rtvHandles[0] = rtvStartHandle;
	device->CreateRenderTargetView(swapChainResources[0].Get(), &rtvDesc, rtvHandles[0]);
	// 先頭からディスクリプタのサイズ分移動した場所のハンドルを取得
	rtvHandles[1].ptr = rtvHandles[0].ptr + device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	// 2つ目を作成
	device->CreateRenderTargetView(swapChainResources[1].Get(), &rtvDesc, rtvHandles[1]);

#pragma endregion

#pragma region DipthStencil Resource & View Create
	ComPtr<ID3D12Resource> depthStencilResource = CreateDepthStencilTextureResource(device, WinApp::kWindoWidth, WinApp::kWindoHeight);

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;	// 基本Resouceにあわせる
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	// DSVHeapの先頭に作成
	device->CreateDepthStencilView(depthStencilResource.Get(), &dsvDesc, dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
#pragma endregion

#pragma region Get & Writing To BackBuffer
	// バックバッファのインデックス取得
	UINT backBufferIndex = swapChain->GetCurrentBackBufferIndex();

#pragma region TransitionBarrier Create
	// TransitionBarrier Settings
	D3D12_RESOURCE_BARRIER barrier{};
	// Type Trainsition
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	// Flag NONE
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	// Barrier To Target
	barrier.Transition.pResource = swapChainResources[backBufferIndex].Get();
	// Before Resource State
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	// After Resource State
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	// Transition Barrier Set
	commandList->ResourceBarrier(1, &barrier);
#pragma endregion

	// 描画先のRTｖをバックバッファのインデックスをもとに設定
	commandList->OMSetRenderTargets(1, &rtvHandles[backBufferIndex], false, nullptr);
	// 画面全体をクリア
	float clearColor[] = { 0.1f, 0.25f, 0.5f, 1.0f };
	commandList->ClearRenderTargetView(rtvHandles[backBufferIndex], clearColor, 0, nullptr);
#pragma endregion

#pragma region DepthStencilHandle Create
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
#pragma endregion

#pragma region TransitionBarrier Change To State
	// Windor Drawing Step
	// State Render -> Present
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	// Transition Barrier Set
	commandList->ResourceBarrier(1, &barrier);
#pragma endregion 

#pragma region CommandList Close & Kick
	// コマンドリスト積込み終了
	hr = commandList->Close();
	assert(SUCCEEDED(hr));
	Log("CloseCommandList\n");

	// GPUにコマンドリストの実行依頼
	ID3D12CommandList* commandLists[] = { commandList.Get() };
	commandQueue->ExecuteCommandLists(1, commandLists);
	// GPUとOSに画面の交換を依頼を通知
	swapChain->Present(1, 0);
#pragma endregion

#pragma region Fence Create
	// Format to 0
	ComPtr<ID3D12Fence> fence = nullptr;
	uint64_t fenceValue = 0;
	hr = device->CreateFence(fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
	assert(SUCCEEDED(hr));
	Log("Create Fence\n");

	// Wait Signal from Fence for Envent
	HANDLE fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	assert(fenceEvent != nullptr);
	Log("Create Fence Event\n");

	// Fence Value Update
	fenceValue++;
	// GPUが実行完了したら指定した値の書き込みをするよう依頼するSignalの送信
	commandQueue->Signal(fence.Get(), fenceValue);

	// Fence Value Check
	if (fence->GetCompletedValue() < fenceValue)
	{
		// Not Completed Wait Event Setting
		fence->SetEventOnCompletion(fenceValue, fenceEvent);
		// Wait Event
		WaitForSingleObject(fenceEvent, INFINITE);
	}
#pragma endregion

#pragma region Next Flame SetUp
	// 次の準備
	hr = commandAllocator->Reset();
	assert(SUCCEEDED(hr));
	hr = commandList->Reset(commandAllocator.Get(), nullptr);
	Log("CommandReset\n");
#pragma endregion

#pragma region dxcCOmplier Initialize
	ComPtr<IDxcUtils> dxcUtils = nullptr;
	ComPtr<IDxcCompiler3> dxcCompiler = nullptr;
	hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils));
	assert(SUCCEEDED(hr));
	hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler));
	assert(SUCCEEDED(hr));
	Log("dxcCompiler Initialized\n");

	// includeに対応するための設定
	ComPtr<IDxcIncludeHandler> includeHandler = nullptr;
	hr = dxcUtils->CreateDefaultIncludeHandler(&includeHandler);
	assert(SUCCEEDED(hr));
#pragma endregion

#pragma region PipeLine Settings

#pragma region RootParameter Create
	D3D12_ROOT_PARAMETER rootParameters[3] = {};
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[0].Descriptor.ShaderRegister = 0;

	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameters[1].Descriptor.ShaderRegister = 0;

	// Texture用
	D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
	descriptorRange[0].BaseShaderRegister = 0;
	descriptorRange[0].NumDescriptors = 1;
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[2].DescriptorTable.pDescriptorRanges = descriptorRange;
	rootParameters[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);
#pragma endregion
#pragma region Smapler Settings
	D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;			// バイナリフィルタ
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;		// 0~1リピート
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;		// 
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;		// 
	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;		// 比較しない
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;						// 最大まで使用
	staticSamplers[0].ShaderRegister = 0;
	staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
#pragma endregion
#pragma region RootSignature Create
	D3D12_ROOT_SIGNATURE_DESC descriptorRootSignature{};
	descriptorRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	descriptorRootSignature.pParameters = rootParameters;
	descriptorRootSignature.NumParameters = _countof(rootParameters);
	descriptorRootSignature.pStaticSamplers = staticSamplers;
	descriptorRootSignature.NumStaticSamplers = _countof(staticSamplers);
	// シリアライズしてバイナリ化
	ComPtr<ID3DBlob> signatureBlob = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	hr = D3D12SerializeRootSignature(&descriptorRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED(hr))
	{
		Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);
	}
	// バイナリを基に生成
	ComPtr<ID3D12RootSignature> rootSignature = nullptr;
	hr = device->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
	assert(SUCCEEDED(hr));
	Log("Created RootSignature\n");
#pragma endregion
#pragma region InputLayout Settings
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[2] = {};
	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs[1].SemanticName = "TEXCOORD";
	inputElementDescs[1].SemanticIndex = 0;
	inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = _countof(inputElementDescs);
#pragma endregion
#pragma region BlendState Settings
	D3D12_BLEND_DESC blendDesc{};
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
#pragma endregion
#pragma region RasterizerState Settings
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	// カリングモード設定
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	// 塗りつぶし
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
#pragma endregion
#pragma region Shader Compile
	const std::string shaderDirectoryPath = "Resources/Shaders/";
	ComPtr<IDxcBlob> vertexShaderBlob = CompileShader(ConvertToWString(shaderDirectoryPath + "Basic3DVS.hlsl"), L"vs_6_0", dxcUtils, dxcCompiler, includeHandler);
	assert(vertexShaderBlob != nullptr);
	ComPtr<IDxcBlob> pixelShaderBlob = CompileShader(ConvertToWString(shaderDirectoryPath + "Basic3DPS.hlsl"), L"ps_6_0", dxcUtils, dxcCompiler, includeHandler);
	assert(pixelShaderBlob != nullptr);
#pragma endregion
#pragma region DepthStencilState Settings
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	depthStencilDesc.DepthEnable = true;							// 深度機能有効化
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;	// 書き込みする
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;	// Depthの値が小さい方が描画される
#pragma endregion
#pragma region PSO Create
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDec{};
	graphicsPipelineStateDec.pRootSignature = rootSignature.Get();
	graphicsPipelineStateDec.InputLayout = inputLayoutDesc;
	graphicsPipelineStateDec.VS = { vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize() };
	graphicsPipelineStateDec.PS = { pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize() };
	graphicsPipelineStateDec.BlendState = blendDesc;
	graphicsPipelineStateDec.RasterizerState = rasterizerDesc;
	// 書き込むRTVの情報
	graphicsPipelineStateDec.NumRenderTargets = 1;
	graphicsPipelineStateDec.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	// 利用するトポロジタイプ
	graphicsPipelineStateDec.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	// 色の打ち込み方設定
	graphicsPipelineStateDec.SampleDesc.Count = 1;
	graphicsPipelineStateDec.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	// 深度情報設定
	graphicsPipelineStateDec.DepthStencilState = depthStencilDesc;
	graphicsPipelineStateDec.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	// 生成
	ComPtr<ID3D12PipelineState> graphicsPipelineState = nullptr;
	hr = device->CreateGraphicsPipelineState(&graphicsPipelineStateDec, IID_PPV_ARGS(&graphicsPipelineState));
	assert(SUCCEEDED(hr));
	Log("Created PSO\n");
#pragma endregion

#pragma endregion

#pragma region TextureResource Create
	// Textureを読み込んで転送
	DirectX::ScratchImage mipImages = LoadTexture("Resources/Textures/uvChecker.png");
	const DirectX::TexMetadata& metaData = mipImages.GetMetadata();
	ComPtr<ID3D12Resource> textureResource = CreateTextureResource(device, metaData);
	ComPtr<ID3D12Resource> intermediateResource = UploadTextureData(textureResource, mipImages, device, commandList);
#pragma endregion
#pragma region テクスチャ転送待ち CommandList Close & Kick & Reset
	// コマンドリスト積込み終了
	hr = commandList->Close();
	assert(SUCCEEDED(hr));

	// GPUにコマンドリストの実行依頼
	commandLists[0] = commandList.Get();
	commandQueue->ExecuteCommandLists(1, commandLists);
	// GPUとOSに画面の交換を依頼を通知
	swapChain->Present(1, 0);

	// Fence Value Update
	fenceValue++;
	// GPUが実行完了したら指定した値の書き込みをするよう依頼するSignalの送信
	commandQueue->Signal(fence.Get(), fenceValue);

	// Fence Value Check
	if (fence->GetCompletedValue() < fenceValue)
	{
		// Not Completed Wait Event Setting
		fence->SetEventOnCompletion(fenceValue, fenceEvent);
		// Wait Event
		WaitForSingleObject(fenceEvent, INFINITE);
	}

	// 次の準備
	hr = commandAllocator->Reset();
	assert(SUCCEEDED(hr));
	hr = commandList->Reset(commandAllocator.Get(), nullptr);
#pragma endregion

#pragma region textureSRV Create
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = metaData.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = static_cast<UINT>(metaData.mipLevels);

	D3D12_CPU_DESCRIPTOR_HANDLE texSrvHandleCPU = srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	D3D12_GPU_DESCRIPTOR_HANDLE texSrvHandleGPU = srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();

	texSrvHandleCPU.ptr += device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	texSrvHandleGPU.ptr += device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	device->CreateShaderResourceView(textureResource.Get(), &srvDesc, texSrvHandleCPU);
#pragma endregion

#pragma region Resources Create
	ComPtr<ID3D12Resource> vertexResourceTriangle = CreataeBufferResource(device, sizeof(VertexData) * 6);
	ComPtr<ID3D12Resource> wvpResourceTriangle = CreataeBufferResource(device, sizeof(Matrix4x4));
	ComPtr<ID3D12Resource> materialResourceTriangle = CreataeBufferResource(device, sizeof(Vector4));

	ComPtr<ID3D12Resource> vertexResourceSprite = CreataeBufferResource(device, sizeof(VertexData) * 6);
	ComPtr<ID3D12Resource> wvpResourceSprite = CreataeBufferResource(device, sizeof(Matrix4x4));
	ComPtr<ID3D12Resource> materialResourceSprite = CreataeBufferResource(device, sizeof(Vector4));
#pragma endregion

#pragma region Resources Writing

#pragma region Trinangle
	VertexData* vertexDataTriangle = nullptr;
	// 書き込み先アドレス取得
	vertexResourceTriangle->Map(0, nullptr, reinterpret_cast<void**>(&vertexDataTriangle));
	vertexDataTriangle[0].position = { -0.5f, -0.5, 0.0f, 1.0f };		// left bottom
	vertexDataTriangle[0].uv = { 0.0f, 1.0f };
	vertexDataTriangle[1].position = { 0.0f, 0.5, 0.0f, 1.0f };			// top
	vertexDataTriangle[1].uv = { 0.5f, 0.0f };
	vertexDataTriangle[2].position = { 0.5f, -0.5, 0.0f, 1.0f };		// right bottom
	vertexDataTriangle[2].uv = { 1.0f, 1.0f };

	vertexDataTriangle[3].position = { -0.5f, -0.5, 0.5f, 1.0f };		// left bottom 2
	vertexDataTriangle[3].uv = { 0.0f, 1.0f };
	vertexDataTriangle[4].position = { 0.0f, 0.0, 0.0f, 1.0f };			// top 2
	vertexDataTriangle[4].uv = { 0.5f, 0.0f };
	vertexDataTriangle[5].position = { 0.5f, -0.5, -0.5f, 1.0f };		// right bottom 2
	vertexDataTriangle[5].uv = { 1.0f, 1.0f };

	Matrix4x4* wvpDataTriangle = nullptr;
	wvpResourceTriangle->Map(0, nullptr, reinterpret_cast<void**>(&wvpDataTriangle));
	*wvpDataTriangle = MakeIdentityMatrix();

	Vector4* materialDataTriangle = nullptr;
	materialResourceTriangle->Map(0, nullptr, reinterpret_cast<void**>(&materialDataTriangle));
	*materialDataTriangle = Vector4(0.0f, 0.5f, 0.5f, 1.0f);
#pragma endregion

#pragma region Sprite
	VertexData* vertexDataSprite = nullptr;
	// 書き込み先アドレス取得
	vertexResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&vertexDataSprite));
	vertexDataSprite[0].position = { 0.0f, static_cast<float>(metaData.height), 0.0f, 1.0f };								// left bottom
	vertexDataSprite[0].uv = { 0.0f, 1.0f };
	vertexDataSprite[1].position = { 0.0f, 0.0f, 0.0f, 1.0f };																// left top
	vertexDataSprite[1].uv = { 0.0f, 0.0f };
	vertexDataSprite[2].position = { static_cast<float>(metaData.width), static_cast<float>(metaData.height), 0.0f, 1.0f };	// right bottom
	vertexDataSprite[2].uv = { 1.0f, 1.0f };

	vertexDataSprite[3].position = { 0.0f, 0.0f, 0.0f, 1.0f };																// left top
	vertexDataSprite[3].uv = { 0.0f, 0.0f };
	vertexDataSprite[4].position = { static_cast<float>(metaData.width), 0.0f, 0.0f, 1.0f };								// right top
	vertexDataSprite[4].uv = { 1.0f, 0.0f };
	vertexDataSprite[5].position = { static_cast<float>(metaData.width), static_cast<float>(metaData.height), 0.0f, 1.0f };	// right bottom	2
	vertexDataSprite[5].uv = { 1.0f, 1.0f };

	Matrix4x4* wvpDataSprite = nullptr;
	wvpResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&wvpDataSprite));
	*wvpDataSprite = MakeIdentityMatrix();

	Vector4* materialDataSprite = nullptr;
	materialResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&materialDataSprite));
	*materialDataSprite = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
#pragma endregion
#pragma endregion

#pragma region VertexBufferView Create
	D3D12_VERTEX_BUFFER_VIEW vertexBufferViewTriangle{};
	vertexBufferViewTriangle.BufferLocation = vertexResourceTriangle->GetGPUVirtualAddress();
	vertexBufferViewTriangle.SizeInBytes = sizeof(VertexData) * 6;
	vertexBufferViewTriangle.StrideInBytes = sizeof(VertexData);

	D3D12_VERTEX_BUFFER_VIEW vertexBufferViewSprite{};
	vertexBufferViewSprite.BufferLocation = vertexResourceSprite->GetGPUVirtualAddress();
	vertexBufferViewSprite.SizeInBytes = sizeof(VertexData) * 6;
	vertexBufferViewSprite.StrideInBytes = sizeof(VertexData);
#pragma endregion

#pragma region Viewport & Scissor
	D3D12_VIEWPORT viewport{};
	// 画面全体に表示
	viewport.Width = WinApp::kWindoWidth;
	viewport.Height = WinApp::kWindoHeight;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	// scissor
	D3D12_RECT scissorRect{};
	// 基本的にはviewportと同じ構成で
	scissorRect.left = 0;
	scissorRect.right = WinApp::kWindoWidth;
	scissorRect.top = 0;
	scissorRect.bottom = WinApp::kWindoHeight;
#pragma endregion

#pragma region ImGui Initialize
	// こういうもん
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(winApp->GetHWND());
	ImGui_ImplDX12_Init(
		device.Get(),
		swapChainDesc.BufferCount,
		rtvDesc.Format, srvDescriptorHeap.Get(),
		srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
		srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
#pragma endregion

#pragma region 変数宣言
	Transform mainCameraTransform = {};
	mainCameraTransform.scale = Vector3(1.0f, 1.0f, 1.0f);
	mainCameraTransform.translate.z = -10.0f;
	Matrix4x4 mainCameraMatrix = MakeAffineMatrix(mainCameraTransform.scale, mainCameraTransform.rotate, mainCameraTransform.translate);
	Matrix4x4 mainCameraViewMatrix = MakeInVerse(mainCameraMatrix);
	Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(0.45f, static_cast<float>(WinApp::kWindoWidth) / static_cast<float>(WinApp::kWindoHeight), 0.1f, 100.0f);

	Matrix4x4 viewMatrix2D = MakeIdentityMatrix();
	Matrix4x4 projectionMatrix2D = MakeOrthoGraphicsMatrix(0.0f, 0.0f, static_cast<float>(WinApp::kWindoWidth), static_cast<float>(WinApp::kWindoHeight), 0.0f, 100.0f);

	Transform triangleTransform = {};
	triangleTransform.scale = Vector3(1.0f, 1.0f, 1.0f);
	Matrix4x4 triangleWorldMatrix = MakeIdentityMatrix();

	Transform spriteTransform = {};
	spriteTransform.scale = Vector3(1.0f, 1.0f, 1.0f);
	Matrix4x4 spriteWorldMatrix = MakeIdentityMatrix();

	Vector4 texColor = *materialDataTriangle;
#pragma endregion

	while (!winApp->PoccesMessage())
	{
#pragma region Begin Frame
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
#pragma endregion

#pragma region Imgui Update
		ImGui::Begin("Debug");
		ImGui::DragFloat4("Tex Color", &texColor.x, 0.001f, 0.0f, 1.0f);
		ImGui::End();

		ImGui::Render();
#pragma endregion

#pragma region GameUpdate
		*materialDataTriangle = texColor;

		triangleTransform.rotate.y += 0.01f;
		triangleWorldMatrix = MakeAffineMatrix(triangleTransform.scale, triangleTransform.rotate, triangleTransform.translate);
		*wvpDataTriangle = triangleWorldMatrix * mainCameraViewMatrix * projectionMatrix;

		spriteWorldMatrix = MakeAffineMatrix(spriteTransform.scale, spriteTransform.rotate, spriteTransform.translate);
		*wvpDataSprite = spriteWorldMatrix * viewMatrix2D * projectionMatrix2D;
#pragma endregion

#pragma region PreDraw
		// バックバッファのインデックス取得
		backBufferIndex = swapChain->GetCurrentBackBufferIndex();
		// Barrier To Target
		barrier.Transition.pResource = swapChainResources[backBufferIndex].Get();
		// Before Resource State
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
		// After Resource State
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
		// Transition Barrier Set
		commandList->ResourceBarrier(1, &barrier);

		// 描画先のRTｖをバックバッファのインデックスをもとに設定
		commandList->OMSetRenderTargets(1, &rtvHandles[backBufferIndex], false, nullptr);
		// 画面全体をクリア
		commandList->ClearRenderTargetView(rtvHandles[backBufferIndex], clearColor, 0, nullptr);

		commandList->OMSetRenderTargets(1, &rtvHandles[backBufferIndex], false, &dsvHandle);
		commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

		ID3D12DescriptorHeap* descriptorHeaps[] = { srvDescriptorHeap.Get() };
		commandList->SetDescriptorHeaps(1, descriptorHeaps);

		commandList->RSSetViewports(1, &viewport);
		commandList->RSSetScissorRects(1, &scissorRect);
		commandList->SetGraphicsRootSignature(rootSignature.Get());
		commandList->SetPipelineState(graphicsPipelineState.Get());
		commandList->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
#pragma endregion

		commandList->IASetVertexBuffers(0, 1, &vertexBufferViewTriangle);
		// CBuffer Set
		commandList->SetGraphicsRootConstantBufferView(1, wvpResourceTriangle->GetGPUVirtualAddress());
		commandList->SetGraphicsRootConstantBufferView(0, materialResourceTriangle->GetGPUVirtualAddress());
		commandList->SetGraphicsRootDescriptorTable(2, texSrvHandleGPU);
		// いざ描画
		commandList->DrawInstanced(6, 1, 0, 0);

		commandList->IASetVertexBuffers(0, 1, &vertexBufferViewSprite);
		// CBuffer Set
		commandList->SetGraphicsRootConstantBufferView(1, wvpResourceSprite->GetGPUVirtualAddress());
		commandList->SetGraphicsRootConstantBufferView(0, materialResourceSprite->GetGPUVirtualAddress());
		commandList->SetGraphicsRootDescriptorTable(2, texSrvHandleGPU);
		// いざ描画
		commandList->DrawInstanced(6, 1, 0, 0);

#pragma region PostDraw
#pragma region ImGui Set
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList.Get());
#pragma endregion

#pragma region TransitionBarrier Change To State
		// Windor Drawing Step
		// State Render -> Present
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
		// Transition Barrier Set
		commandList->ResourceBarrier(1, &barrier);
#pragma endregion
#pragma region CommandList Close & Kick
		// コマンドリスト積込み終了
		hr = commandList->Close();
		assert(SUCCEEDED(hr));

		// GPUにコマンドリストの実行依頼
		commandLists[0] = commandList.Get();
		commandQueue->ExecuteCommandLists(1, commandLists);
		// GPUとOSに画面の交換を依頼を通知
		swapChain->Present(1, 0);
#pragma endregion
#pragma region Fence Wait
		// Fence Value Update
		fenceValue++;
		// GPUが実行完了したら指定した値の書き込みをするよう依頼するSignalの送信
		commandQueue->Signal(fence.Get(), fenceValue);

		// Fence Value Check
		if (fence->GetCompletedValue() < fenceValue)
		{
			// Not Completed Wait Event Setting
			fence->SetEventOnCompletion(fenceValue, fenceEvent);
			// Wait Event
			WaitForSingleObject(fenceEvent, INFINITE);
		}
#pragma endregion
#pragma region Next Flame SetUp
		// 次の準備
		hr = commandAllocator->Reset();
		assert(SUCCEEDED(hr));
		hr = commandList->Reset(commandAllocator.Get(), nullptr);
#pragma endregion

#pragma endregion
	}
#pragma region Finalize
	CloseHandle(fenceEvent);
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	materialResourceSprite->Unmap(0, nullptr);
	wvpResourceSprite->Unmap(0, nullptr);
	vertexResourceSprite->Unmap(0, nullptr);

	materialResourceTriangle->Unmap(0, nullptr);
	wvpResourceTriangle->Unmap(0, nullptr);
	vertexResourceTriangle->Unmap(0, nullptr);

	winApp->Finalize();
	// COM終了
	CoUninitialize();
#pragma endregion

	return 0;
}

/// <summary>
/// シェーダコンパイル
/// </summary>
/// <param name="filePath">CompilerするShaderファイルへのパス</param>
/// <param name="peofile">Complerに使用するProfile</param>
/// <param name="dxcUtils"></param>
/// <param name="dxcCompiler"></param>
/// <param name="includeHandler"></param>
/// <returns></returns>
ComPtr<IDxcBlob> CompileShader(const std::wstring& filePath, const wchar_t* profile, const ComPtr<IDxcUtils>& dxcUtils, const ComPtr<IDxcCompiler3>& dxcCompiler, const ComPtr<IDxcIncludeHandler>& includeHandler)
{
#pragma region HLSL Loading
	Log(ConvertToString(std::format(L"Begin CompileShader, path:{}, profile:{}\n", filePath, profile)));
	// hlsl Load
	ComPtr<IDxcBlobEncoding> shaderSource = nullptr;
	HRESULT hr = dxcUtils->LoadFile(filePath.c_str(), nullptr, &shaderSource);
	assert(SUCCEEDED(hr));
	Log("Shader Load Complete");

	// 内容設定
	DxcBuffer shaderSourceBuffer;
	shaderSourceBuffer.Ptr = shaderSource->GetBufferPointer();
	shaderSourceBuffer.Size = shaderSource->GetBufferSize();
	shaderSourceBuffer.Encoding = DXC_CP_UTF8;	// UTF-8のコードであることを通知
#pragma endregion
#pragma region Compiler
	LPCWSTR arguments[] = {
		filePath.c_str(),			// コンパイルするhlslファイル名
		L"-E", L"main",				// エントリーポイント指定
		L"-T", profile,				// ShaderProfile設定
		L"-Zi", L"-Qembed_debug",	// デバッグ用の情報埋め込み
		L"-Od",						// 最適化はしない
		L"-Zpr",					// メモリレイアウトは行優先
	};
	// Shader Compile
	ComPtr<IDxcResult> shaderResult = nullptr;
	hr = dxcCompiler->Compile(
		&shaderSourceBuffer,		// 読み込んだファイル
		arguments,					// コンパイル時のオプション
		_countof(arguments),		// ↑の数
		includeHandler.Get(),		// Includeとか
		IID_PPV_ARGS(&shaderResult)	// 結果
	);
	assert(SUCCEEDED(hr));

	// 問題が発生したらログに出力
	ComPtr<IDxcBlobUtf8> shaderError = nullptr;
	shaderResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&shaderError), nullptr);
	if (shaderError != nullptr && shaderError->GetStringLength() != 0)
	{
		// ログを出して停止
		Log(shaderError->GetStringPointer());
		shaderError->Release();
		assert(false);
	}
#pragma endregion
#pragma region Result
	ComPtr<IDxcBlob> shaderBlob = nullptr;
	hr = shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr);
	assert(SUCCEEDED(hr));
	Log(ConvertToString(std::format(L"Complie Succeeded, path:{}, profile:{}\n", filePath, profile)));
	// Release
	shaderSource->Release();
	shaderResult->Release();
#pragma endregion
	return shaderBlob;
}

/// <summary>
/// リソース作成関数
/// </summary>
/// <param name="device">作成してくれるデバイス</param>
/// <param name="sizeInBytes">使用サイズ</param>
/// <returns></returns>
ComPtr<ID3D12Resource> CreataeBufferResource(const ComPtr<ID3D12Device>& device, size_t sizeInBytes)
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
	HRESULT hr = device->CreateCommittedResource(&uploadHeapPoperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&resource));
	assert(SUCCEEDED(hr));
	Log("Created Resource\n");
	return resource;
}

/// <summary>
/// デスクリプタヒープ作成関数
/// </summary>
/// <param name="device">作成してくれるデバイス</param>
/// <param name="heapType">作成するタイプ</param>
/// <param name="numDescriptors">デスクリプタ個数</param>
/// <param name="shaderVisible"></param>
/// <returns></returns>
ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(const ComPtr<ID3D12Device>& device, D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible)
{
	ID3D12DescriptorHeap* descriptorHeap = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
	descriptorHeapDesc.Type = heapType;
	descriptorHeapDesc.NumDescriptors = numDescriptors;
	descriptorHeapDesc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	HRESULT hr = device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&descriptorHeap));
	// ディスクリプタヒープ生成確認
	assert(SUCCEEDED(hr));
	Log("CreateDecritorHeap\n");
	return descriptorHeap;
}

/// <summary>
/// ミップマップ付きデータの取得
/// </summary>
/// <param name="filePath"></param>
/// <returns></returns>
DirectX::ScratchImage LoadTexture(const std::string& filePath)
{
	// TextureFileえおプログラム用に読み込む
	DirectX::ScratchImage image{};
	std::wstring filePathW = ConvertToWString(filePath);
	HRESULT hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
	assert(SUCCEEDED(hr));
	Log("Texture Load\n");

	DirectX::ScratchImage mipImage{};
	hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImage);
	Log("MipMap Create\n");
	return mipImage;
}

/// <summary>
/// テクスチャリソースの作成
/// </summary>
/// <param name="device">作成してくれるデバイス</param>
/// <param name="metaData">作成元データ</param>
/// <returns></returns>
ComPtr<ID3D12Resource> CreateTextureResource(const ComPtr<ID3D12Device>& device, const DirectX::TexMetadata& metaData)
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
	Log("TextureResource Created\n");
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
[[nodiscard]]
ComPtr<ID3D12Resource> UploadTextureData(const ComPtr<ID3D12Resource>& texture, const DirectX::ScratchImage& mipImage, const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12GraphicsCommandList>& commandList)
{
	// 中間リソースの作成までを別関数にわかるべきか？
	std::vector<D3D12_SUBRESOURCE_DATA> subresources;
	DirectX::PrepareUpload(device.Get(), mipImage.GetImages(), mipImage.GetImageCount(), mipImage.GetMetadata(), subresources);
	uint64_t intermediateSize = GetRequiredIntermediateSize(texture.Get(), 0, static_cast<UINT>(subresources.size()));
	ComPtr<ID3D12Resource> intermediateResource = CreataeBufferResource(device, intermediateSize);

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
	Log("MipMap Upload To Texture\n");
	return intermediateResource;
}

/// <summary>
/// 深度バッファ用リソースの作成
/// </summary>
/// <param name="device"></param>
/// <param name="width"></param>
/// <param name="height"></param>
/// <returns></returns>
ComPtr<ID3D12Resource> CreateDepthStencilTextureResource(const ComPtr<ID3D12Device>& device, int32_t width, int32_t height)
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
	HRESULT hr = device->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE, // データ転送設定
		&depthClearValue,
		IID_PPV_ARGS(&resource)
	);
	assert(SUCCEEDED(hr));
	Log("TextureResource Created\n");
	return resource;
}