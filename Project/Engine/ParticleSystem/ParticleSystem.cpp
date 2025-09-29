#include "ParticleSystem.h"

#include <cassert>
#include <numbers>

#include "DirectXCommon.h"
#include "DxCommand.h"
#include "DxDevice.h"
#include "DxShader.h"
#include "DirectX12ObjectsFunction.h"
#include "TextureManager.h"
#include "Logger.h"
#include "Random.h"
#include "BasicShapes/Triangle.h"

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
	ID3D12GraphicsCommandList *commandList = dxCommon_->GetCommand()->GetCommandList();

	commandList->SetGraphicsRootSignature(rootSignature_.Get());
	commandList->SetPipelineState(pipelineStateObject_.Get());
	//triangle_->GetVertexbufferView();
	triangle_->Draw();
	commandList->SetGraphicsRootDescriptorTable(0, particleForGPUSrvGpuHandle_);
	commandList->SetGraphicsRootDescriptorTable(1, texHandleGPU_);

	commandList->DrawInstanced(triangle_->GetVerticiesNum(), currentInstanceNum_, 0, 0);
}

void ParticleSystem::Emit(const Vector3 &position, uint32_t emitCount)
{
	for(uint32_t i = 0 ; i < emitCount; i++)
	{
		Particle newParticle = MakeParticle();
		newParticle.transform.translate += position;
		particles_.push_back(newParticle);

		if(particles_.size() >= kMaxParticles)
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
#pragma region RootParameter Create
	D3D12_ROOT_PARAMETER rootParameters[2] = {};


	D3D12_DESCRIPTOR_RANGE desctiptorRangeForInstancing[1]{};
	desctiptorRangeForInstancing[0].BaseShaderRegister = 0;
	desctiptorRangeForInstancing[0].NumDescriptors = 1;
	desctiptorRangeForInstancing[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	desctiptorRangeForInstancing[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameters[0].DescriptorTable.pDescriptorRanges = desctiptorRangeForInstancing;
	rootParameters[0].DescriptorTable.NumDescriptorRanges = _countof(desctiptorRangeForInstancing);


	// Texture用
	D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
	descriptorRange[0].BaseShaderRegister = 0;
	descriptorRange[0].NumDescriptors = 1;
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[1].DescriptorTable.pDescriptorRanges = descriptorRange;
	rootParameters[1].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);
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
		Logger::Log(reinterpret_cast<char *>(errorBlob->GetBufferPointer()));
		assert(false);
	}
	hr = dxCommon_->GetDxDevice()->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature_));
	assert(SUCCEEDED(hr));
	Logger::Log("Created RootSignature\n");
#pragma endregion
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
	D3D12_BLEND_DESC blendDesc = DirectX12ObjectsFunction::InitializeBlendMode(BlendMode::ADD);

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
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;	// 書き込みする
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

void ParticleSystem::CreateSRV()
{
	D3D12_SHADER_RESOURCE_VIEW_DESC instancingDesc4TransformationMatrix{};
	instancingDesc4TransformationMatrix.Format = DXGI_FORMAT_UNKNOWN;
	instancingDesc4TransformationMatrix.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	instancingDesc4TransformationMatrix.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	instancingDesc4TransformationMatrix.Buffer.FirstElement = 0;
	instancingDesc4TransformationMatrix.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	instancingDesc4TransformationMatrix.Buffer.NumElements = kMaxParticles;
	instancingDesc4TransformationMatrix.Buffer.StructureByteStride = sizeof(TransformationMatrix);

	D3D12_CPU_DESCRIPTOR_HANDLE srvCpuHandle = dxCommon_->GetSRVDescriptorCPUHandle(3);
	particleForGPUSrvGpuHandle_ = dxCommon_->GetSRVDescriptorGPUHandle(3);
	dxCommon_->GetDxDevice()->GetDevice()->CreateShaderResourceView(transformationMatrixResource_.Get(), &instancingDesc4TransformationMatrix, srvCpuHandle);
}