#pragma once
#include <d3d12.h>

#include "ComPtr.h"

class DirectXCommon;
class DxRootSignature;
class DxInputlayout;

class DxPipelineStateObject
{
public:
	/*DxPipelineStateObject(
		ID3D12Device* device,
		ID3D12RootSignature* rootSignature,
		const D3D12_INPUT_LAYOUT_DESC& inputLayout, );
	*/

private:

	ComPtr<ID3D12PipelineState> pipelineStateObject_ = nullptr;
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc_{};
};