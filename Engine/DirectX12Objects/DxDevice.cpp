#include "DxDevice.h"

#include <cassert>
#include <format>

#include "../Logger.h"
#include "../StringUtility.h"

// DirectX12
#pragma comment(lib, "dxgi.lib")
// Debug
#pragma comment(lib, "dxguid.lib")

void DxDevice::Initialize()
{
#ifdef _DEBUG
	ComPtr<ID3D12Debug1> debugController = nullptr;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
		// デバッグレイヤー有効化
		debugController->EnableDebugLayer();
		// GPU側でもチェックを開始
		debugController->SetEnableGPUBasedValidation(TRUE);
	}
#endif

	// エラーコードの取得
	HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory_));
	// SUCCEEDEDマクロで判定
	assert(SUCCEEDED(hr));

	// 性能順で選別
	for (UINT i = 0;
		dxgiFactory_->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&useAdapter_)) != DXGI_ERROR_NOT_FOUND;
		++i)
	{
		// 情報習得
		DXGI_ADAPTER_DESC3 adapterDesc{};
		hr = useAdapter_->GetDesc3(&adapterDesc);
		assert(SUCCEEDED(hr));

		// ソフトウェア以外であれば採用
		if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE))
		{
			// 使用するアダプタをログに出力
			Logger::Log(StringUtility::ConvertToString(std::format(L"Use Adapter:{}\n", adapterDesc.Description)));
			break;
		}
		useAdapter_ = nullptr;
	}

	assert(useAdapter_ != nullptr);

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
		hr = D3D12CreateDevice(useAdapter_.Get(), featureLevels[i], IID_PPV_ARGS(&device_));
		// 指定した機能レベルで生成できたか確認
		if (SUCCEEDED(hr))
		{
			// 生成できていればループ終了
			Logger::Log(std::format("FeatureLevel : {}\n", featureLevelStrings[i]));
			break;
		}
	}
	// デバイスが生成できているか確認
	assert(device_ != nullptr);
	Logger::Log("Complete Create D3D12Device\n");

#ifdef _DEBUG
	ComPtr<ID3D12InfoQueue> infoQueue = nullptr;
	if (SUCCEEDED(device_->QueryInterface(IID_PPV_ARGS(&infoQueue)))) {
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
}

ID3D12Device4* DxDevice::GetDevice() const
{
	return device_.Get();
}

IDXGIFactory7* DxDevice::GetFactory() const
{
	return dxgiFactory_.Get();
}
