#pragma once

#include <d3d12.h>
#include <dxcapi.h>
#include <memory>

#include "../ComPtr.h"

class DxDevice;
class DxRootSignature;
class DxShaderManager;

class DxPipelineStateObject
{
public:

	//void DefaultInitialize(DxDevice*);
	void InitializeAndCreate(DxDevice* device, DxRootSignature* rootSignature, IDxcBlob* ps, IDxcBlob* vs);
	//ID3D12RootSignature* GetRootSignature();
	ID3D12PipelineState* GetPipelineState();
	//UINT GetRootParamIndex(const std::string&);

private:

	ComPtr<ID3D12PipelineState> graphicsPipelineState = nullptr;
};