#include "DxPipelineStateObject.h"
#include "DxShader.h"
#include "DxInputLayout.h"
//#include "DxBlendMode.h"
#include "DxObjFunctions.h"
#include "Logger.h"

#include <cassert>

DxPipelineStateObject::DxPipelineStateObject(
	ID3D12Device4 *device,
	ID3D12RootSignature *rootSignature,
	const D3D12_INPUT_LAYOUT_DESC &inputLayout,
	const std::string &shaderGroupName)
{
	// Shaders Compile
	DxShaderCompiler::ShaderGroup shaders = DxShaderCompiler::CompileShaderGroup(shaderGroupName);

	// BlendState Settings
	D3D12_BLEND_DESC blendDesc = DxObjFunctions::InitializeBlendMode(BlendMode::ALPHA);

	// RasterizerState Settings
	D3D12_RASTERIZER_DESC rasterizerDesc = DxObjFunctions::InitializeRasterizerState();

	// DepthStencilState Settings
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc = DxObjFunctions::InitializeDepthStencilState(DepthMode::LessEqual);

	// PSO Create
	mPsoDesc_.pRootSignature = rootSignature;
	mPsoDesc_.InputLayout = inputLayout;
	mPsoDesc_.VS = { shaders.vs->GetBufferPointer(), shaders.vs->GetBufferSize() };
	mPsoDesc_.PS = { shaders.ps->GetBufferPointer(), shaders.ps->GetBufferSize() };
	mPsoDesc_.BlendState = blendDesc;
	mPsoDesc_.RasterizerState = rasterizerDesc;
	// 書き込むRTVの情報
	mPsoDesc_.NumRenderTargets = 1;
	mPsoDesc_.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	// 利用するトポロジタイプ
	mPsoDesc_.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	// 色の打ち込み方設定
	mPsoDesc_.SampleDesc.Count = 1;
	mPsoDesc_.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	// 深度情報設定
	mPsoDesc_.DepthStencilState = depthStencilDesc;
	mPsoDesc_.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	// 生成
	HRESULT hr = device->CreateGraphicsPipelineState(&mPsoDesc_, IID_PPV_ARGS(&mPipelineStateObject_));
	assert(SUCCEEDED(hr));
	Logger::Log("Created PSO\n");
}
