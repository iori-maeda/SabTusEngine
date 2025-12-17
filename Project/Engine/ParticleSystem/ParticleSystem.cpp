#include "ParticleSystem.h"

#include <cassert>
#include <numbers>

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
	cmdList->DrawInstanced(triangle_->GetVerticiesNum(), currentInstanceNum_, 0, 0);
}

void ParticleSystem::Emit(const Vector3 &position, uint32_t emitCount)
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


	dxRootSignature_->AddRootParamSemantic(ParamSemanticType::Particle, ParamType::DescriptorTable, ShaderVisibility::Vertex, 0, 1)
		.AddRootParamSemantic(ParamSemanticType::Texture, ParamType::DescriptorTable, ShaderVisibility::Pixel, 0, 1);

	dxRootSignature_->Create(dxCommon_->GetDxDevice()->GetDevice());
}

void ParticleSystem::CreatePipelineStateObject()
{

	// Shader Compile
	DxShaderCompiler::ShaderGroup shaders = DxShaderCompiler::CompileShaderGroup("BasicParticle");

	// InputLayout Settings
	DxInputLayout dxInputLayout;
	dxInputLayout.AddLayout(LayoutSemanthicType::Position,LayoutFormat::FLOAT4,0)
		.AddLayout(LayoutSemanthicType::Texcoord, LayoutFormat::FLOAT2,0);

	DxPipelineStateObjectBuilder psoBuilder;

	// PSO Create
	dxPipelineStateObject_ = psoBuilder
		.SetRootSignature(dxRootSignature_->GetRootSignature())
		.SetInputLayout(dxInputLayout.GetLayoutDesc())
		.SetShaderGroup("BasicParticle")
		.SetBlendMode(BlendMode::ADD)
		.SetDepthStencilState(DepthMode::LessEqual,true,false)
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