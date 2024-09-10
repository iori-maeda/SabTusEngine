#pragma once

#include <d3d12.h>

#include "../ComPtr.h"

class DxDevice;

class DxPipelineStateObject
{
public:

	void DefaultInitialize(DxDevice*);
	void Initialize();

	ID3D12RootSignature* GetRootSignature();
	ID3D12PipelineState* GetPipelineState();

private:

	ComPtr<ID3D12RootSignature> rootSignature = nullptr;
	ComPtr<ID3D12PipelineState> graphicsPipelineState = nullptr;
};