#pragma once
#include <unordered_map>
#include <memory>
#include <d3d12.h>
#include <string>

#include "DxPipelineStateObject.h"
#include "DxObjFunctions.h"

class DxPipelineStateObjectBuilder
{
public:
	DxPipelineStateObjectBuilder() = default;

	DxPipelineStateObjectBuilder& SetBlendMode(BlendMode blendMode);
	DxPipelineStateObjectBuilder& SetRasterizerState(CullingMode cullMode, bool isSolid = true);
	DxPipelineStateObjectBuilder& SetDepthStencilState(DepthMode depthMode, bool isDepthTestEnable = true, bool isDepthWriteEnable = true);
	DxPipelineStateObjectBuilder& SetRootSignature(ID3D12RootSignature* rootSignature);
	DxPipelineStateObjectBuilder& SetInputLayout(const D3D12_INPUT_LAYOUT_DESC& inputLayout);
	DxPipelineStateObjectBuilder& SetShaderGroup(const std::string& shaderGroupName);


	std::unique_ptr<DxPipelineStateObject> Build(ID3D12Device4* device);


private:
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc_{};
	DxShaderCompiler::ShaderGroup shaders_{};
};

