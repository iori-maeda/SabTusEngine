#pragma once
#include <d3d12.h>
#include <string>

#include "ComPtr.h"

class DxBlendMode;

class DxPipelineStateObject
{
public:
	DxPipelineStateObject() = delete;
	DxPipelineStateObject(
		ID3D12Device4* device,
		ID3D12RootSignature* rootSignature,
		const D3D12_INPUT_LAYOUT_DESC& inputLayout,
		const std::string& shaderGroupName
	);
	DxPipelineStateObject(ID3D12Device4* device, const D3D12_GRAPHICS_PIPELINE_STATE_DESC& desc);

public:
	ID3D12PipelineState* GetPipelineStateObject() const { return mPipelineStateObject_.Get(); }

private:
	ComPtr<ID3D12PipelineState> mPipelineStateObject_ = nullptr;
	D3D12_GRAPHICS_PIPELINE_STATE_DESC mPsoDesc_{};
};