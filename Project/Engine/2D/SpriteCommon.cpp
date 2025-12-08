#include "SpriteCommon.h"
#include "DxDevice.h"
#include "DxCommand.h"
#include "DxShader.h"
#include "DxRootSignature.h"
#include "Logger.h" 
#include "DxObjFunctions.h"
#include "DxInputLayout.h"


void SpriteCommon::Initialize(DirectXCommon *dxCommon)
{
	dxCommon_ = dxCommon;
	CreateRootSignature();
	CreatePipelineStateObject();
}

void SpriteCommon::PreDraw()
{
	dxCommon_->GetCommand()->GetCommandList()->SetGraphicsRootSignature(dxRootSignature_->GetRootSignature());
	dxCommon_->GetCommand()->GetCommandList()->SetPipelineState(pipelineStateObject_.Get());
}

void SpriteCommon::CreateRootSignature()
{
	dxRootSignature_ = std::make_unique<DxRootSignature>();

	dxRootSignature_->AddRootParamSemantic(
		ParamSemanticType::TextureMaterial,
		ParamType::CBV,
		ShaderVisibility::Pixel,
		0
	);

	dxRootSignature_->AddRootParamSemantic(
		ParamSemanticType::TransformationMatrix,
		ParamType::CBV,
		ShaderVisibility::Vertex,
		0
	);

	dxRootSignature_->AddRootParamSemantic(
		ParamSemanticType::Texture,
		ParamType::DescriptorTable,
		ShaderVisibility::Pixel,
		0,
		1
	);

	dxRootSignature_->Initialize(dxCommon_->GetDxDevice()->GetDevice());
}

void SpriteCommon::CreatePipelineStateObject()
{

#pragma region Shader Compile
	const std::string shaderDirectoryPath = "Resources/Shaders/";
	ComPtr<IDxcBlob> vertexShaderBlob = DxShaderCompiler::GetInstancxe().CompileShader(shaderDirectoryPath + "Basic2DVS.hlsl", L"vs_6_0");
	assert(vertexShaderBlob != nullptr);
	ComPtr<IDxcBlob> pixelShaderBlob = DxShaderCompiler::GetInstancxe().CompileShader(shaderDirectoryPath + "Basic2DPS.hlsl", L"ps_6_0");
	assert(pixelShaderBlob != nullptr);
#pragma endregion

#pragma region InputLayout Settings
	DxInputLayout inputLayout;
	inputLayout.AddLayout(LayoutSemanthicType::Position, LayoutFormat::FLOAT4, 0)
		.AddLayout(LayoutSemanthicType::Texcoord, LayoutFormat::FLOAT2, 0);
#pragma endregion

	// BlendState Settings
	D3D12_BLEND_DESC blendDesc = DxObjFunctions::InitializeBlendMode(BlendMode::ALPHA);

#pragma region RasterizerState Settings
	D3D12_RASTERIZER_DESC rasterizerDesc = DxObjFunctions::InitializeRasterizerState();
#pragma endregion

#pragma region DepthStencilState Settings
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc = DxObjFunctions::InitializeDepthStencilState();
#pragma endregion

#pragma region PSO Create
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDec{};
	graphicsPipelineStateDec.pRootSignature = dxRootSignature_->GetRootSignature();
	graphicsPipelineStateDec.InputLayout = inputLayout.GetLayoutDesc();
	graphicsPipelineStateDec.VS = { vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize() };
	graphicsPipelineStateDec.PS = { pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize() };
	graphicsPipelineStateDec.BlendState = blendDesc;
	graphicsPipelineStateDec.RasterizerState = rasterizerDesc;
	// 書き込むRTVの情報
	graphicsPipelineStateDec.NumRenderTargets = 1;
	graphicsPipelineStateDec.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	// 利用するトポロジタイプ
	graphicsPipelineStateDec.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	// 色の打ち込み方設定
	graphicsPipelineStateDec.SampleDesc.Count = 1;
	graphicsPipelineStateDec.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	// 深度情報設定
	graphicsPipelineStateDec.DepthStencilState = depthStencilDesc;
	graphicsPipelineStateDec.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	// 生成
	HRESULT hr = dxCommon_->GetDxDevice()->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDec, IID_PPV_ARGS(&pipelineStateObject_));
	assert(SUCCEEDED(hr));
	Logger::Log("SpriteRenderer Created PSO\n");
#pragma endregion
}