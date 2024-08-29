#pragma once

#include "../ComPtr.h"

#include <d3d12.h>

#include <dxgi1_6.h>
#pragma comment(lib, "dxgi.lib")
// Debug
#include <dxgidebug.h>
#pragma comment(lib, "dxguid.lib")

class DxDevice
{
public:
	void Initialize();

	ID3D12Device4* GetDevice();
	IDXGIFactory7* GetFactory();

private:
	struct DxResourceLeakChecker
	{
		~DxResourceLeakChecker()
		{
			// Resources Leak Check
			ComPtr<IDXGIDebug1> debug;
			if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debug))))
			{
				debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
				debug->ReportLiveObjects(DXGI_DEBUG_APP, DXGI_DEBUG_RLO_ALL);
				debug->ReportLiveObjects(DXGI_DEBUG_D3D12, DXGI_DEBUG_RLO_ALL);
			}
		};
	};
	DxResourceLeakChecker leakCheck_;

	ComPtr<ID3D12Debug1> debugController_ = nullptr;
	ComPtr<IDXGIFactory7> dxgiFactory_ = nullptr;
	ComPtr<IDXGIAdapter4> useAdapter_ = nullptr;
	ComPtr<ID3D12Device4> device_ = nullptr;
};