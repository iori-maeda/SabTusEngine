#include "DxPipelineStateObjectBuilder.h"
#include "DxShader.h"


DxPipelineStateObjectBuilder& DxPipelineStateObjectBuilder::SetBlendMode(BlendMode blendMode)
{
	psoDesc_.BlendState = DxObjFunctions::InitializeBlendMode(blendMode);
	return *this;
}

DxPipelineStateObjectBuilder& DxPipelineStateObjectBuilder::SetRasterizerState(CullingMode cullMode, bool isSolid)
{
	psoDesc_.RasterizerState = DxObjFunctions::InitializeRasterizerState(cullMode, isSolid);
	return *this;
}

DxPipelineStateObjectBuilder& DxPipelineStateObjectBuilder::SetDepthStencilState(DepthMode depthMode, bool isDepthTestEnable, bool isDepthWriteEnable)
{
	psoDesc_.DepthStencilState = DxObjFunctions::InitializeDepthStencilState(depthMode, isDepthTestEnable, isDepthWriteEnable);
	return *this;
}

DxPipelineStateObjectBuilder& DxPipelineStateObjectBuilder::SetRootSignature(ID3D12RootSignature* rootSignature)
{
	psoDesc_.pRootSignature = rootSignature;
	return *this;
}
DxPipelineStateObjectBuilder& DxPipelineStateObjectBuilder::SetInputLayout(const D3D12_INPUT_LAYOUT_DESC& inputLayout)
{
	psoDesc_.InputLayout = inputLayout;
	return *this;
}
DxPipelineStateObjectBuilder& DxPipelineStateObjectBuilder::SetShaderGroup(const std::string& shaderGroupName)
{
	shaders_ = DxShaderCompiler::CompileShaderGroup(shaderGroupName);

	// TODO: return ステートメントをここに挿入します
	psoDesc_.VS = { shaders_.vs->GetBufferPointer(), shaders_.vs->GetBufferSize() };
	psoDesc_.PS = { shaders_.ps->GetBufferPointer(), shaders_.ps->GetBufferSize() };
	return *this;
}

std::unique_ptr<DxPipelineStateObject> DxPipelineStateObjectBuilder::Build(ID3D12Device4* device)
{
	// 書き込むRTVの情報
	psoDesc_.NumRenderTargets = 1;
	psoDesc_.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	// 利用するトポロジタイプ
	psoDesc_.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	// 色の打ち込み方設定
	psoDesc_.SampleDesc.Count = 1;
	psoDesc_.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	// 深度情報設定
	psoDesc_.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	std::unique_ptr<DxPipelineStateObject> builtPso = std::make_unique<DxPipelineStateObject>(device, psoDesc_);
	shaders_ = {};
	return std::move(builtPso);
}
