#include "Object3dCommon.h"

#include <numbers>

#include "DxDevice.h"
#include "DxCommand.h"
#include "DxShader.h"
#include "DxRootSignature.h"
#include "DxInputLayout.h"
#include "Logger.h" 
#include "DxObjFunctions.h"
#include "ImGuiManager.h"
#include "ModelManager.h"
#include "Lights.h"

Object3dCommon::~Object3dCommon()
{}

void Object3dCommon::Initialize(DirectXCommon *dxCommon)
{
	dxCommon_ = dxCommon;

	ModelManager::GetInstace().Initialize(dxCommon_);

	CreateRootSignature();
	CreatePipelineStateObject();

	essentialResource_ = dxCommon_->CreateBufferResource(sizeof(Essential));
	essentialResource_->Map(0, nullptr, reinterpret_cast<void **>(&essentialForGPUData_));

	fogResource_ = dxCommon_->CreateBufferResource(sizeof(FogStatus));
	fogResource_->Map(0, nullptr, reinterpret_cast<void **>(&fogData_));
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
	ImGui::End();
#endif 
}

void Object3dCommon::PreDraw()
{
	essentialForGPUData_->numLights = lights_->GetLightsNum();

	// 描画コマンドリストの取得
	ID3D12GraphicsCommandList *commandList = dxCommon_->GetCommand()->GetCommandList();

	commandList->SetGraphicsRootSignature(dxRootSignature_->GetRootSignature());
	commandList->SetPipelineState(pipelineStateObject_.Get());
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
		.AddRootParamSemantic(ParamSemanticType::Texture, ParamType::DescriptorTable, ShaderVisibility::Pixel, 0, 1)
		.AddRootParamSemantic(ParamSemanticType::ObjectEssential, ParamType::CBV, ShaderVisibility::Pixel, 1)
		.AddRootParamSemantic(ParamSemanticType::CameraTransform, ParamType::CBV, ShaderVisibility::Pixel, 2)
		.AddRootParamSemantic(ParamSemanticType::Lights, ParamType::DescriptorTable, ShaderVisibility::Pixel, 1, 1)
		.AddRootParamSemantic(ParamSemanticType::ObjectMaterial, ParamType::CBV, ShaderVisibility::Pixel, 3)
		.AddRootParamSemantic(ParamSemanticType::Fog, ParamType::CBV, ShaderVisibility::Pixel, 4);

	dxRootSignature_->Create(dxCommon_->GetDxDevice()->GetDevice());
}

void Object3dCommon::CreatePipelineStateObject()
{
#pragma region Shader Compile
	DxShaderCompiler::ShaderGroup shaders = DxShaderCompiler::CompileShaderGroup("Basic3d");
#pragma endregion

#pragma region InputLayout Settings
	DxInputLayout inputLayoutDesc;
	inputLayoutDesc.AddLayout(LayoutSemanthicType::Position, LayoutFormat::FLOAT4, 0)
		.AddLayout(LayoutSemanthicType::Texcoord, LayoutFormat::FLOAT2, 0)
		.AddLayout(LayoutSemanthicType::Normal, LayoutFormat::FLOAT3, 0);
#pragma endregion

	// BlendState Settings
	D3D12_BLEND_DESC blendDesc = DxObjFunctions::InitializeBlendMode(BlendMode::ALPHA);

#pragma region RasterizerState Settings
	D3D12_RASTERIZER_DESC rasterizerDesc = DxObjFunctions::InitializeRasterizerState();
#pragma endregion

#pragma region DepthStencilState Settings
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc = DxObjFunctions::InitializeDepthStencilState(DepthMode::LessEqual);
#pragma endregion

#pragma region PSO Create
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDec{};
	graphicsPipelineStateDec.pRootSignature = dxRootSignature_->GetRootSignature();
	graphicsPipelineStateDec.InputLayout = inputLayoutDesc.GetLayoutDesc();
	graphicsPipelineStateDec.VS = { shaders.vs->GetBufferPointer(), shaders.vs->GetBufferSize() };
	graphicsPipelineStateDec.PS = { shaders.ps->GetBufferPointer(), shaders.ps->GetBufferSize() };
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