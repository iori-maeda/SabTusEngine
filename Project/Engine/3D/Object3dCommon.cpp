#include "Object3dCommon.h"

#include <numbers>

#include "DxDevice.h"
#include "DxCommand.h"
#include "DxShader.h"
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

	
}

void Object3dCommon::DebugWindow()
{
#ifdef USE_IMGUI
	/*ImGui::Begin("Object3dCommon");
	ImGui::DragFloat3("DirectionalLight Direction", &directionalLightData_->direction.x, 0.01f);
	ImGui::ColorEdit4("DirectionalLight Color", &directionalLightData_->color.x);
	ImGui::SliderFloat("DirectionalLight Intensity", &directionalLightData_->intensity, 0.0f, 100.0f);

	ImGui::DragFloat3("PointLight position", &pointLightData_->position.x, 0.01f);
	ImGui::ColorEdit4("PointLight Color", &pointLightData_->color.x);
	ImGui::DragFloat("PointLight Intensity", &pointLightData_->intensity, 0.01f);
	ImGui::SliderFloat("PointLight Radius", &pointLightData_->radius, 0.00f, 100.0f);
	ImGui::SliderFloat("PointLight Decay", &pointLightData_->decay, 0.0f, 100.0f);

	ImGui::DragFloat3("SpotLight position", &spotLightData_->position.x, 0.01f);
	ImGui::DragFloat3("SpotLight Direction", &spotLightData_->direction.x, 0.01f);
	ImGui::ColorEdit4("SpotLight Color", &spotLightData_->color.x);
	ImGui::SliderFloat("SpotLight Intensity", &spotLightData_->intensity, 0.0f, 100.0f);
	ImGui::SliderFloat("SpotLight Distance", &spotLightData_->distance, 0.00f, 100.0f);
	ImGui::SliderFloat("SpotLight Decay", &spotLightData_->decay, 0.0f, 100.0f);
	ImGui::SliderFloat("SpotLight CosStartFallOff", &spotLightData_->cosFallOffStart, 0.0f, 1.0f);
	ImGui::SliderFloat("SpotLight CosAngle", &spotLightData_->cosAngle, 0.0f, 1.0f);
	ImGui::End();

	directionalLightData_->direction = Normalize(directionalLightData_->direction);
	spotLightData_->direction = Normalize(spotLightData_->direction);*/
#endif 
}

void Object3dCommon::PreDraw()
{
	// 描画コマンドリストの取得
	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommand()->GetCommandList();

	commandList->SetGraphicsRootSignature(rootSignature_.Get());
	commandList->SetPipelineState(pipelineStateObject_.Get());
	lights_->SetDrawCommand();

	
}

void Object3dCommon::CreateRootSignature()
{
#pragma region RootParameter Create
	D3D12_ROOT_PARAMETER rootParameters[7] = {};

	// Material
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[0].Descriptor.ShaderRegister = 0;

	// TransformationMatrix
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameters[1].Descriptor.ShaderRegister = 0;

	// Texture
	D3D12_DESCRIPTOR_RANGE descriptorRangeTexture[1] = {};
	descriptorRangeTexture[0].BaseShaderRegister = 0;
	descriptorRangeTexture[0].NumDescriptors = 1;
	descriptorRangeTexture[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRangeTexture[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[2].DescriptorTable.pDescriptorRanges = descriptorRangeTexture;
	rootParameters[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRangeTexture);

	// Essential
	rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[3].Descriptor.ShaderRegister = 1;

	// Camera
	rootParameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[4].Descriptor.ShaderRegister = 2;

	// Lights
	D3D12_DESCRIPTOR_RANGE descriptorRangeLight[1] = {};
	descriptorRangeLight[0].BaseShaderRegister = 1;
	descriptorRangeLight[0].NumDescriptors = 1;
	descriptorRangeLight[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRangeLight[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	rootParameters[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[5].DescriptorTable.pDescriptorRanges = descriptorRangeLight;
	rootParameters[5].DescriptorTable.NumDescriptorRanges = _countof(descriptorRangeLight);

	//// Spot Light
	//D3D12_DESCRIPTOR_RANGE descriptorRangeSpotLight[1] = {};
	//descriptorRangeSpotLight[0].BaseShaderRegister = descriptorRangePointLight[0].NumDescriptors;
	//descriptorRangeSpotLight[0].NumDescriptors = Lights::sMaxSpotLights;
	//descriptorRangeSpotLight[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	//descriptorRangeSpotLight[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	//rootParameters[6].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	//rootParameters[6].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	//rootParameters[6].DescriptorTable.pDescriptorRanges = descriptorRangeSpotLight;
	//rootParameters[6].DescriptorTable.NumDescriptorRanges = _countof(descriptorRangeSpotLight);
#pragma endregion
#pragma region Smapler Settings
	D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;			// バイナリフィルタ
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;		// 0~1リピート
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;		// 
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;		// 
	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;		// 比較しない
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;						// 最大まで使用
	staticSamplers[0].ShaderRegister = 0;
	staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
#pragma endregion
#pragma region RootSignature Create
	D3D12_ROOT_SIGNATURE_DESC descriptorRootSignature{};
	descriptorRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	descriptorRootSignature.pParameters = rootParameters;
	descriptorRootSignature.NumParameters = _countof(rootParameters);
	descriptorRootSignature.pStaticSamplers = staticSamplers;
	descriptorRootSignature.NumStaticSamplers = _countof(staticSamplers);
	// シリアライズしてバイナリ化
	ComPtr<ID3DBlob> signatureBlob = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&descriptorRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED(hr))
	{
		Logger::Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);
	}
	hr = dxCommon_->GetDxDevice()->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature_));
	assert(SUCCEEDED(hr));
	Logger::Log("Created RootSignature\n");
#pragma endregion
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
	graphicsPipelineStateDec.pRootSignature = rootSignature_.Get();
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