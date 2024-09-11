#pragma once

#include <d3d12.h>
#include <memory>

#include "../ComPtr.h"
#include "DxRootSignature.h"

class DxDevice;

class DxPipelineStateObject
{
public:

	void DefaultInitialize(DxDevice*);
	void Initialize();

	ID3D12RootSignature* GetRootSignature();
	ID3D12PipelineState* GetPipelineState();
	UINT GetRootParamIndex(const std::string&);

private:

	ComPtr<ID3D12PipelineState> graphicsPipelineState = nullptr;
	std::unique_ptr<DxRootSignature> rootSignature_ = nullptr;
};