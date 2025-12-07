#include "ParticleSystem.h"

#include <cassert>
#include <numbers>

#include "DirectXCommon.h"
#include "DxCommand.h"
#include "DxDevice.h"
#include "DxShader.h"
#include "DxRootSignature.h"
#include "DxObjFunctions.h"
#include "TextureManager.h"
#include "Logger.h"
#include "Random.h"
#include "BasicShapes/Triangle.h"

ParticleSystem* ParticleSystem::instance_ = nullptr;

ParticleSystem* ParticleSystem::GetInstance()
{
	if (instance_ == nullptr) { instance_ = new ParticleSystem; }
	return instance_;
}

void ParticleSystem::Initialize(DirectXCommon* dxCommon)
{
	assert(dxCommon);
	dxCommon_ = dxCommon;

	triangle_ = new Triangle();
	triangle_->Initialize(dxCommon);

	transformationMatrixResource_ = dxCommon_->CreateBufferResource(sizeof(ParticleForGPU) * kMaxParticles);
	transformationMatrixResource_->Map(0, nullptr, reinterpret_cast<void**>(&particleForGPUData_));
	materialResource_ = dxCommon_->CreateBufferResource(sizeof(MaterialData));
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
	materialData_->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	for (int i = 0; i < kMaxParticles; i++)
	{
		particles_.push_back(MakeParticle());
	}

	CreateSRV();
	CreateRootSignature();
	CreatePipelineStateObject();


	TextureManager::GetInstace().Initialize(dxCommon);
	TextureManager::GetInstace().Load("uvChecker.png");
	texHandleGPU_ = TextureManager::GetInstace().GetSRVDescriptorGPUHandle("uvChecker.png");

	accelerationField_.acceleration = Vector3(-100.0f, 0.0f, 0.0f);
	accelerationField_.area = { .min = {-1.0f,-1.0f, -1.0f},  .max = {1.0f, 1.0f, 1.0f} };
}

void ParticleSystem::Finalize()
{
	if (instance_ == nullptr) { return; }
	delete instance_;
}

ParticleSystem::~ParticleSystem()
{
	delete triangle_;
}


void ParticleSystem::Update()
{
	//if (camera_) { triangle_->SetCamera(camera_); }

	currentInstanceNum_ = 0;
	for (std::list<Particle>::iterator particleIterator = particles_.begin(); particleIterator != particles_.end();)
	{
		float deltaTIme = 1.0f / 60.0f;

		// 生存時間を過ぎていたらスキップ
		if (particleIterator->currentTime > particleIterator->lifeTIme)
		{
			// 削除して次のiteratorへ
			particleIterator = particles_.erase(particleIterator);
			continue;
		}

		if (particleIterator->transform.translate.x >= accelerationField_.area.min.x && particleIterator->transform.translate.x <= accelerationField_.area.max.x
			&& particleIterator->transform.translate.y >= accelerationField_.area.min.y && particleIterator->transform.translate.y <= accelerationField_.area.max.y
			&& particleIterator->transform.translate.z >= accelerationField_.area.min.z && particleIterator->transform.translate.z <= accelerationField_.area.max.z
			)
		{
			particleIterator->velocity += accelerationField_.acceleration * deltaTIme;
		}

		particleIterator->currentTime += deltaTIme;
		particleIterator->transform.translate += particleIterator->velocity * deltaTIme;
		particleIterator->color.w = 1.0f - (particleIterator->currentTime / particleIterator->lifeTIme);

		particleForGPUData_[currentInstanceNum_].world = MakeAffineMatrix(particleIterator->transform.scale, particleIterator->transform.rotate, particleIterator->transform.translate);
		particleForGPUData_[currentInstanceNum_].wvp = MakeIdentityMatrix();
		particleForGPUData_[currentInstanceNum_].color = particleIterator->color;
		if (camera_ == nullptr) { return; }

		Matrix4x4 billboardMatrix = /*MakeRotateY(std::numbers::pi_v<float>) **/ camera_->GetWorldMatrix();
		billboardMatrix.m[3][0] = 0.0f;
		billboardMatrix.m[3][1] = 0.0f;
		billboardMatrix.m[3][2] = 0.0f;
		particleForGPUData_[currentInstanceNum_].world = MakeScaleMatrix(particleIterator->transform.scale) * billboardMatrix * MakeTranslateMatrix(particleIterator->transform.translate);
		particleForGPUData_[currentInstanceNum_].wvp = particleForGPUData_[currentInstanceNum_].world * camera_->GetViewMatrix() * camera_->GetProjectionMatrix();;

		currentInstanceNum_++;
		particleIterator++;
	}
}

void ParticleSystem::Draw()
{
	ID3D12GraphicsCommandList* cmdList = dxCommon_->GetCommand()->GetCommandList();

	cmdList->SetGraphicsRootSignature(dxRootSignature_->GetRootSignature());
	cmdList->SetPipelineState(pipelineStateObject_.Get());
	//triangle_->GetVertexbufferView();
	triangle_->Draw();
	cmdList->SetGraphicsRootDescriptorTable(
		dxRootSignature_->GetRootParamIndex(DxRootSignature::ParamSemanticType::Particle),
		particleForGPUSrvGpuHandle_
	);

	cmdList->SetGraphicsRootDescriptorTable(
		dxRootSignature_->GetRootParamIndex(DxRootSignature::ParamSemanticType::Texture),
		texHandleGPU_
	);
	cmdList->DrawInstanced(triangle_->GetVerticiesNum(), currentInstanceNum_, 0, 0);
}

void ParticleSystem::Emit(const Vector3& position, uint32_t emitCount)
{
	for (uint32_t i = 0; i < emitCount; i++)
	{
		Particle newParticle = MakeParticle();
		newParticle.transform.translate += position;
		particles_.push_back(newParticle);

		if (particles_.size() >= kMaxParticles)
		{
			particles_.pop_front();
		}
	}
}

ParticleSystem::Particle ParticleSystem::MakeParticle()
{
	Particle particle{};

	particle.transform.scale = Vector3(Random::GetRandom(0.01f, 0.1f), Random::GetRandom(0.01f, 0.1f), Random::GetRandom(0.01f, 0.1f));
	particle.transform.rotate = Vector3(Random::GetRandom(0.01f, 0.1f), Random::GetRandom(0.01f, 0.1f), Random::GetRandom(0.01f, 0.1f));
	particle.transform.translate = Vector3(Random::GetRandom(-0.5f, 0.5f), Random::GetRandom(-0.5f, 0.5f), Random::GetRandom(-0.5f, 0.5f));
	particle.velocity = Vector3(Random::GetRandom(-5.0, 5.0f), Random::GetRandom(-5.0, 5.0f), Random::GetRandom(-5.0, 5.0f));
	particle.lifeTIme = Random::GetRandom(1.0f, 3.0f);
	particle.color = Vector4(Random::GetRandom(0.0f, 1.0f), Random::GetRandom(0.0f, 1.0f), Random::GetRandom(0.0f, 1.0f), Random::GetRandom(0.5f, 1.0f));

	return particle;
}

void ParticleSystem::CreateRootSignature()
{
	dxRootSignature_ = std::make_unique<DxRootSignature>();


	dxRootSignature_->AddRootParamSemantic(
		DxRootSignature::ParamSemanticType::Particle,
		DxRootSignature::ParamType::DescriptorTable,
		DxRootSignature::ShaderVisibility::Vertex,
		0,
		1
	);

	dxRootSignature_->AddRootParamSemantic(
		DxRootSignature::ParamSemanticType::Texture,
		DxRootSignature::ParamType::DescriptorTable,
		DxRootSignature::ShaderVisibility::Pixel,
		0,
		1
	);

	dxRootSignature_->Initialize(dxCommon_->GetDxDevice()->GetDevice());
}

void ParticleSystem::CreatePipelineStateObject()
{

#pragma region Shader Compile
	const std::string shaderDirectoryPath = "Resources/Shaders/";
	ComPtr<IDxcBlob> vertexShaderBlob = DxShaderCompiler::GetInstancxe().CompileShader(shaderDirectoryPath + "BasicParticle.VS.hlsl", L"vs_6_0");
	assert(vertexShaderBlob != nullptr);
	ComPtr<IDxcBlob> pixelShaderBlob = DxShaderCompiler::GetInstancxe().CompileShader(shaderDirectoryPath + "BasicParticle.PS.hlsl", L"ps_6_0");
	assert(pixelShaderBlob != nullptr);
#pragma endregion

#pragma region InputLayout Settings
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[2] = {};
	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs[1].SemanticName = "TEXCOORD";
	inputElementDescs[1].SemanticIndex = 0;
	inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = _countof(inputElementDescs);
#pragma endregion

	// BlendState Settings
	D3D12_BLEND_DESC blendDesc = DxObjFunctions::InitializeBlendMode(BlendMode::ADD);

#pragma region RasterizerState Settings
	D3D12_RASTERIZER_DESC rasterizerDesc = DxObjFunctions::InitializeRasterizerState();
#pragma endregion

#pragma region DepthStencilState Settings
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc = DxObjFunctions::InitializeDepthStencilState();
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

void ParticleSystem::CreateSRV()
{
	D3D12_SHADER_RESOURCE_VIEW_DESC instancingDesc4TransformationMatrix{};
	instancingDesc4TransformationMatrix.Format = DXGI_FORMAT_UNKNOWN;
	instancingDesc4TransformationMatrix.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	instancingDesc4TransformationMatrix.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	instancingDesc4TransformationMatrix.Buffer.FirstElement = 0;
	instancingDesc4TransformationMatrix.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	instancingDesc4TransformationMatrix.Buffer.NumElements = kMaxParticles;
	instancingDesc4TransformationMatrix.Buffer.StructureByteStride = sizeof(ParticleForGPU);

	D3D12_CPU_DESCRIPTOR_HANDLE srvCpuHandle = dxCommon_->GetSRVDescriptorCPUHandle(1);
	particleForGPUSrvGpuHandle_ = dxCommon_->GetSRVDescriptorGPUHandle(1);
	dxCommon_->GetDxDevice()->GetDevice()->CreateShaderResourceView(transformationMatrixResource_.Get(), &instancingDesc4TransformationMatrix, srvCpuHandle);
}