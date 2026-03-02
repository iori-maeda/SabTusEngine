#include "Object3dCommon.h"

#include <numbers>

#include "DxDevice.h"
#include "DxCommand.h"
#include "DxShader.h"
#include "DxRootSignature.h"
#include "DxInputLayout.h"
#include "DxPipelineStateObjectBuilder.h"
#include "Logger.h" 
#include "DxObjFunctions.h"
#include "ImGuiManager.h"
#include "ModelManager.h"
#include "Lights.h"

Object3dCommon::~Object3dCommon()
{
}

void Object3dCommon::Initialize(DirectXCommon* dxCommon)
{
	dxCommon_ = dxCommon;

	ModelManager::GetInstace().Initialize(dxCommon_);

	CreateRootSignature();
	CreatePipelineStateObject();

	essentialResource_ = dxCommon_->CreateBufferResource(sizeof(Essential));
	essentialResource_->Map(0, nullptr, reinterpret_cast<void**>(&essentialForGPUData_));

	fogResource_ = dxCommon_->CreateBufferResource(sizeof(FogStatus));
	fogResource_->Map(0, nullptr, reinterpret_cast<void**>(&fogData_));
	*fogData_ = FogStatus();
}

void Object3dCommon::DebugWindow()
{
#ifdef USE_IMGUI
	lights_->DebugWindow();

	ImGui::Begin("Fog Debug Windo");
	ImGui::DragFloat("Density", &fogData_->density, 0.0001f, 0.0f, 1.0f);
	ImGui::DragFloat("Power", &fogData_->power, 0.01f, 0.0f, 100.0f);
	ImGui::DragFloat("Threshold Start", &fogData_->thresholdStart, 0.0001f, 0.0f, 1.0f);
	ImGui::DragFloat("Threshold End", &fogData_->thresholdEnd, 0.0001f, 0.0f, 1.0f);
	bool usePBR = static_cast<bool>(essentialForGPUData_->drawPBR);
	ImGui::Checkbox("Use NormalTex", &usePBR);
	essentialForGPUData_->drawPBR = static_cast<int>(usePBR);
	ImGui::End();
#endif 
}

void Object3dCommon::PreDraw()
{
	essentialForGPUData_->numLights = lights_->GetLightsNum();

	// 描画コマンドリストの取得
	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommand()->GetCommandList();

	commandList->SetGraphicsRootSignature(dxRootSignature_->GetRootSignature());
	commandList->SetPipelineState(dxPipelineStateObject_->GetPipelineStateObject());
	commandList->SetGraphicsRootConstantBufferView(
		dxRootSignature_->GetRootParamIndex(ParamSemanticType::ObjectEssential),
		essentialResource_->GetGPUVirtualAddress()
	);
	lights_->SetParamindex(dxRootSignature_->GetRootParamIndex(ParamSemanticType::Lights));
	lights_->DrawCommandSet();
	commandList->SetGraphicsRootConstantBufferView(
		dxRootSignature_->GetRootParamIndex(ParamSemanticType::Fog),
		fogResource_->GetGPUVirtualAddress()
	);
}

void Object3dCommon::CreateRootSignature()
{
	dxRootSignature_ = std::make_unique<DxRootSignature>();

	dxRootSignature_->AddRootParamSemantic(ParamSemanticType::MeshMaterial, ParamType::CBV, ShaderVisibility::Pixel, 0)
		.AddRootParamSemantic(ParamSemanticType::TransformationMatrix, ParamType::CBV, ShaderVisibility::Vertex, 0)
		.AddRootParamSemantic(ParamSemanticType::ObjectEssential, ParamType::CBV, ShaderVisibility::Pixel, 1)
		.AddRootParamSemantic(ParamSemanticType::CameraTransform, ParamType::CBV, ShaderVisibility::Pixel, 2)
		.AddRootParamSemantic(ParamSemanticType::Lights, ParamType::DescriptorTable, ShaderVisibility::Pixel, 0, 1)
		.AddRootParamSemantic(ParamSemanticType::Texture, ParamType::DescriptorTable, ShaderVisibility::Pixel, 1, 4)
		.AddRootParamSemantic(ParamSemanticType::ObjectMaterial, ParamType::CBV, ShaderVisibility::Pixel, 3)
		.AddRootParamSemantic(ParamSemanticType::Fog, ParamType::CBV, ShaderVisibility::Pixel, 4);

	dxRootSignature_->Create(dxCommon_->GetDxDevice()->GetDevice());
}

void Object3dCommon::CreatePipelineStateObject()
{
	// InputLayout Settings
	DxInputLayout inputLayoutDesc;
	inputLayoutDesc.AddLayout(LayoutSemanthicType::Position, LayoutFormat::FLOAT4, 0)
		.AddLayout(LayoutSemanthicType::Texcoord, LayoutFormat::FLOAT2, 0)
		.AddLayout(LayoutSemanthicType::Normal, LayoutFormat::FLOAT3, 0)
		.AddLayout(LayoutSemanthicType::Tangernt, LayoutFormat::FLOAT3, 0);

	/*DxBlendMode dxBlendModes;
	dxBlendModes.AddUseMode(BlendMode::ALPHA);*/

	DxPipelineStateObjectBuilder psoBuilder;

	// PSO Create
	dxPipelineStateObject_ = psoBuilder
		.SetRootSignature(dxRootSignature_->GetRootSignature())
		.SetInputLayout(inputLayoutDesc.GetLayoutDesc())
		.SetShaderGroup("Basic3d")
		.SetBlendMode(BlendMode::ALPHA)
		.SetRasterizerState(CullingMode::Back)
		.SetDepthStencilState(DepthMode::LessEqual)
		.Build(dxCommon_->GetDxDevice()->GetDevice());
}