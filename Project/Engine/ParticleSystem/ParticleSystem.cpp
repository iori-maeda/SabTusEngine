#include "ParticleSystem.h"

#include <cassert>
#include <numbers>
#include <format>

#include "DirectXCommon.h"
#include "DxCommand.h"
#include "DxDevice.h"
#include "DxShader.h"
#include "DxRootSignature.h"
#include "DxInputLayout.h"
#include "DxObjFunctions.h"
#include "TextureManager.h"
#include "Logger.h"
#include "Random.h"
#include "BasicShapes/Triangle.h"
#include "DxPipelineStateObjectBuilder.h"


ParticleSystem *ParticleSystem::instance_ = nullptr;

ParticleSystem *ParticleSystem::GetInstance()
{
	if (instance_ == nullptr) { instance_ = new ParticleSystem; }
	return instance_;
}

void ParticleSystem::Initialize(DirectXCommon *dxCommon)
{
	assert(dxCommon);
	dxCommon_ = dxCommon;

	triangle_ = new Triangle();
	triangle_->Initialize(dxCommon);

	particleEssentialResource_ = dxCommon_->CreateBufferResource(sizeof(ParticleEssential));
	particleEssentialResource_->Map(0, nullptr, reinterpret_cast<void **>(&particleEssentialData_));
	transformationMatrixResource_ = dxCommon_->CreateBufferResource(sizeof(ParticleForGPU) * kMaxParticles);
	transformationMatrixResource_->Map(0, nullptr, reinterpret_cast<void **>(&particleForGPUData_));
	materialResource_ = dxCommon_->CreateBufferResource(sizeof(MaterialData));
	materialResource_->Map(0, nullptr, reinterpret_cast<void **>(&materialData_));
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
	//Logger::Log(std::format("max count {}\n", kMaxParticles));
	currentInstanceNum_ = 0;
	for (Particle &particle : particles_)
	{
		float deltaTIme = 1.0f / 60.0f;

		// 生存時間を過ぎていたらスキップ
		if (particle.currentTime > particle.lifeTIme)
		{
			// 削除して次のiteratorへ
			//particleIterator = particles_.erase(particleIterator);
			particle = particles_.back();
			particles_.pop_back();
			continue;
		}

		// 加速範囲内であれば加速
		if (particle.transform.translate.x >= accelerationField_.area.min.x && particle.transform.translate.x <= accelerationField_.area.max.x
			&& particle.transform.translate.y >= accelerationField_.area.min.y && particle.transform.translate.y <= accelerationField_.area.max.y
			&& particle.transform.translate.z >= accelerationField_.area.min.z && particle.transform.translate.z <= accelerationField_.area.max.z
			)
		{
			particle.velocity += accelerationField_.acceleration * deltaTIme;
		}

		particle.currentTime += deltaTIme;


		particle.transform.translate += particle.velocity * deltaTIme;
		particle.color.w = 1.0f - (particle.currentTime / particle.lifeTIme);

		particleForGPUData_[currentInstanceNum_].transfotm = particle.transform;
		particleForGPUData_[currentInstanceNum_].color = particle.color;

		currentInstanceNum_++;
	}
	if (camera_ == nullptr) { return; }
	particleEssentialData_->camera.position = camera_->GetPosition();
	particleEssentialData_->camera.viewMat = camera_->GetViewMatrix();
	particleEssentialData_->camera.projeMat = camera_->GetProjectionMatrix();
}

void ParticleSystem::Draw()
{
	ID3D12GraphicsCommandList *cmdList = dxCommon_->GetCommand()->GetCommandList();

	cmdList->SetGraphicsRootSignature(dxRootSignature_->GetRootSignature());
	cmdList->SetPipelineState(dxPipelineStateObject_->GetPipelineStateObject());
	//triangle_->GetVertexbufferView();
	triangle_->Draw();
	cmdList->SetGraphicsRootDescriptorTable(
		dxRootSignature_->GetRootParamIndex(ParamSemanticType::Particle),
		particleForGPUSrvGpuHandle_
	);

	cmdList->SetGraphicsRootDescriptorTable(
		dxRootSignature_->GetRootParamIndex(ParamSemanticType::Texture),
		texHandleGPU_
	);

	cmdList->SetGraphicsRootConstantBufferView(
		dxRootSignature_->GetRootParamIndex(ParamSemanticType::CameraTransform),
		particleEssentialResource_->GetGPUVirtualAddress()
	);

	cmdList->DrawInstanced(triangle_->GetVerticiesNum(), currentInstanceNum_, 0, 0);
}

void ParticleSystem::Emit(const Vector3 &position, uint32_t emitCount)
{
	//if (particles_.size() >= kMaxParticles - 1) { return; }

	for (uint32_t i = 0; i < emitCount; i++)
	{
		Particle newParticle = MakeParticle();
		newParticle.transform.translate += position;
		particles_.push_back(newParticle);

		if (particles_.size() >= kMaxParticles)
		{
			break;
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


	dxRootSignature_->AddRootParamSemantic(ParamSemanticType::Particle, ParamType::DescriptorTable, ShaderVisibility::Vertex, 0, 1)
		.AddRootParamSemantic(ParamSemanticType::Texture, ParamType::DescriptorTable, ShaderVisibility::Pixel, 0, 1)
		.AddRootParamSemantic(ParamSemanticType::CameraTransform, ParamType::CBV, ShaderVisibility::Vertex, 0, 1);

	dxRootSignature_->Create(dxCommon_->GetDxDevice()->GetDevice());
}

void ParticleSystem::CreatePipelineStateObject()
{

	// Shader Compile
	DxShaderCompiler::ShaderGroup shaders = DxShaderCompiler::CompileShaderGroup("BasicParticle");

	// InputLayout Settings
	DxInputLayout dxInputLayout;
	dxInputLayout.AddLayout(LayoutSemanthicType::Position, LayoutFormat::FLOAT4, 0)
		.AddLayout(LayoutSemanthicType::Texcoord, LayoutFormat::FLOAT2, 0);

	DxPipelineStateObjectBuilder psoBuilder;

	// PSO Create
	dxPipelineStateObject_ = psoBuilder
		.SetRootSignature(dxRootSignature_->GetRootSignature())
		.SetInputLayout(dxInputLayout.GetLayoutDesc())
		.SetShaderGroup("BasicParticle")
		.SetBlendMode(BlendMode::ADD)
		.SetDepthStencilState(DepthMode::LessEqual, true, false)
		.SetRasterizerState(CullingMode::Back)
		.Build(dxCommon_->GetDxDevice()->GetDevice());
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