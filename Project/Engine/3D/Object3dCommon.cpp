#include "Object3dCommon.h"

#include <numbers>

#include "DxDevice.h"
#include "DxCommand.h"
#include "DxShader.h"
#include "DxRootSignature.h"
#include "Logger.h" 
#include "DirectX12ObjectsFunction.h"
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
}

void Object3dCommon::DebugWindow()
{
#ifdef USE_IMGUI

#endif 
}

void Object3dCommon::PreDraw()
{
	essentialForGPUData_->numLights = lights_->GetLightsNum();

	// 描画コマンドリストの取得
	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommand()->GetCommandList();

	commandList->SetGraphicsRootSignature(dxRootSignature_->GetRootSignature());
	commandList->SetPipelineState(pipelineStateObject_.Get());
	commandList->SetGraphicsRootConstantBufferView(
		dxRootSignature_->GetRootParamIndex(DxRootSignature::ParamSemanticType::Essential),
		essentialResource_->GetGPUVirtualAddress()
	);
}

void Object3dCommon::CreateRootSignature()
{
	dxRootSignature_ = std::make_unique<DxRootSignature>();

	dxRootSignature_->AddRootParamSemantic(
		DxRootSignature::ParamSemanticType::MeshMaterial,
		DxRootSignature::ParamType::CBV,
		DxRootSignature::ShaderVisibility::Pixel,
		0
	);

	dxRootSignature_->AddRootParamSemantic(
		DxRootSignature::ParamSemanticType::TransformationMatrix,
		DxRootSignature::ParamType::CBV,
		DxRootSignature::ShaderVisibility::Vertex,
		0
	);
	
	dxRootSignature_->AddRootParamSemantic(
		DxRootSignature::ParamSemanticType::Texture,
		DxRootSignature::ParamType::DescriptorTable,
		DxRootSignature::ShaderVisibility::Pixel,
		0,
		1
	);

	dxRootSignature_->AddRootParamSemantic(
		DxRootSignature::ParamSemanticType::Essential,
		DxRootSignature::ParamType::CBV,
		DxRootSignature::ShaderVisibility::Pixel,
		1
	);

	dxRootSignature_->AddRootParamSemantic(
		DxRootSignature::ParamSemanticType::CameraTransform,
		DxRootSignature::ParamType::CBV,
		DxRootSignature::ShaderVisibility::Pixel,
		2
	);

	dxRootSignature_->AddRootParamSemantic(
		DxRootSignature::ParamSemanticType::Lights,
		DxRootSignature::ParamType::DescriptorTable,
		DxRootSignature::ShaderVisibility::Pixel,
		1,
		1
	);
	
	dxRootSignature_->AddRootParamSemantic(
		DxRootSignature::ParamSemanticType::ObjectMaterial,
		DxRootSignature::ParamType::CBV,
		DxRootSignature::ShaderVisibility::Pixel,
		3
	);

	dxRootSignature_->Initialize(dxCommon_->GetDxDevice()->GetDevice());
}

void Object3dCommon::CreatePipelineStateObject()
{

#pragma region Shader Compile
	const std::string shaderDirectoryPath = "Resources/Shaders/";
	ComPtr<IDxcBlob> vertexShaderBlob = DxShaderCompiler::GetInstancxe().CompileShader(shaderDirectoryPath + "Basic3DVS.hlsl", L"vs_6_0");
	assert(vertexShaderBlob != nullptr);
	ComPtr<IDxcBlob> pixelShaderBlob = DxShaderCompiler::GetInstancxe().CompileShader(shaderDirectoryPath + "Basic3DPS.hlsl", L"ps_6_0");
	assert(pixelShaderBlob != nullptr);
#pragma endregion

#pragma region InputLayout Settings
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[3] = {};
	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs[1].SemanticName = "TEXCOORD";
	inputElementDescs[1].SemanticIndex = 0;
	inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs[2].SemanticName = "NORMAL";
	inputElementDescs[2].SemanticIndex = 0;
	inputElementDescs[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDescs[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = _countof(inputElementDescs);
#pragma endregion

	// BlendState Settings
	D3D12_BLEND_DESC blendDesc = DirectX12ObjectsFunction::InitializeBlendMode(BlendMode::NONE);

#pragma region RasterizerState Settings
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	// カリングモード設定
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	// 塗りつぶし
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
#pragma endregion

#pragma region DepthStencilState Settings
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	depthStencilDesc.DepthEnable = true;							// 深度機能有効化
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;	// 書き込みする
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;	// Depthの値が小さい方が描画される
#pragma endregion

#pragma region PSO Create
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDec{};
	graphicsPipelineStateDec.pRootSignature = dxRootSignature_->GetRootSignature();
	graphicsPipelineStateDec.InputLayout = inputLayoutDesc;
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