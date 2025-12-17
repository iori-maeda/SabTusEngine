#include "SpriteCommon.h"
#include "DxDevice.h"
#include "DxCommand.h"
#include "DxShader.h"
#include "DxRootSignature.h"
#include "Logger.h" 
#include "DxObjFunctions.h"
#include "DxInputLayout.h"
#include "DxPipelineStateObjectBuilder.h"

void SpriteCommon::Initialize(DirectXCommon *dxCommon)
{
	dxCommon_ = dxCommon;
	CreateRootSignature();
	CreatePipelineStateObject();
}

void SpriteCommon::PreDraw()
{
	dxCommon_->GetCommand()->GetCommandList()->SetGraphicsRootSignature(dxRootSignature_->GetRootSignature());
	dxCommon_->GetCommand()->GetCommandList()->SetPipelineState(dxPipelineStateObject_->GetPipelineStateObject());
}

void SpriteCommon::CreateRootSignature()
{
	dxRootSignature_ = std::make_unique<DxRootSignature>();

	dxRootSignature_->AddRootParamSemantic(ParamSemanticType::TextureMaterial, ParamType::CBV, ShaderVisibility::Pixel, 0)
		.AddRootParamSemantic(ParamSemanticType::TransformationMatrix, ParamType::CBV, ShaderVisibility::Vertex, 0)
		.AddRootParamSemantic(ParamSemanticType::Texture, ParamType::DescriptorTable, ShaderVisibility::Pixel, 0, 1);

	dxRootSignature_->Create(dxCommon_->GetDxDevice()->GetDevice());
}

void SpriteCommon::CreatePipelineStateObject()
{

	// Shader Compile
	DxShaderCompiler::ShaderGroup shaders = DxShaderCompiler::CompileShaderGroup("Basic2d");

	// InputLayout Settings
	DxInputLayout inputLayout;
	inputLayout.AddLayout(LayoutSemanthicType::Position, LayoutFormat::FLOAT4, 0)
		.AddLayout(LayoutSemanthicType::Texcoord, LayoutFormat::FLOAT2, 0);

	DxPipelineStateObjectBuilder psoBuilder;

	// PSO Create
	dxPipelineStateObject_ = psoBuilder
		.SetRootSignature(dxRootSignature_->GetRootSignature())
		.SetInputLayout(inputLayout.GetLayoutDesc())
		.SetShaderGroup("Basic2d")
		.SetBlendMode(BlendMode::ALPHA)
		.SetRasterizerState(CullingMode::Back)
		.SetDepthStencilState(DepthMode::None)
		.Build(dxCommon_->GetDxDevice()->GetDevice());
}