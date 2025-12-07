#pragma once
#include <d3d12.h>
#include <list>
#include <memory>

#include "ComPtr.h"
#include "Math/Vector2.h"
#include "Math/Vector3.h"
#include "Math/Vector4.h"
#include "Math/Matrix4x4.h"
#include "Camera/Camera.h"

class DirectXCommon;
class DxRootSignature;
class Triangle;

class ParticleSystem
{
public:
	static const int kMaxParticles = 4096;

	struct  Transform
	{
		Vector3 scale{ 1.0f, 1.0f, 1.0f };
		Vector3 rotate{};
		Vector3 translate{};
	};

	struct Particle
	{
		Transform transform{};
		Vector3 velocity{};
		Vector4 color{ 1.0f, 1.0f, 1.0f, 1.0f };
		float lifeTIme = 1.0f;
		float currentTime = 0.0f;
	};

	struct TransformationMatrix
	{
		Matrix4x4 wvp{};
		Matrix4x4 world{};
	};

	struct MaterialData
	{
		Vector4 color{};
	};

	struct ParticleForGPU
	{
		Matrix4x4 wvp{};
		Matrix4x4 world{};
		Vector4 color{};
	};

	struct AABB
	{
		Vector3 min{};
		Vector3 max{ 1.0f, 1.0f, 1.0f };
	};

	struct AccelerationField
	{
		Vector3 acceleration{};
		AABB area{};
	};

public:
	static ParticleSystem *GetInstance();
	void Initialize(DirectXCommon *dxCommon);
	void Finalize();
	void Update();
	void Draw();

	void Emit(const Vector3 &position, uint32_t emitCount);
	ParticleSystem::Particle MakeParticle();

public:
	void SetCamera(Camera *camera) { camera_ = camera; }

private:
	ParticleSystem() = default;
	~ParticleSystem();
	void CreateRootSignature();
	void CreatePipelineStateObject();
	void CreateSRV();

private:
	static ParticleSystem *instance_;
	DirectXCommon *dxCommon_ = nullptr;
	Camera *camera_ = nullptr;

	std::unique_ptr<DxRootSignature> dxRootSignature_ = nullptr;
	ComPtr<ID3D12PipelineState> pipelineStateObject_ = nullptr;

	ComPtr<ID3D12Resource> transformationMatrixResource_ = nullptr;
	ParticleForGPU *particleForGPUData_ = nullptr;

	ComPtr<ID3D12Resource> materialResource_ = nullptr;
	MaterialData *materialData_ = nullptr;

	D3D12_GPU_DESCRIPTOR_HANDLE particleForGPUSrvGpuHandle_{};
	//D3D12_GPU_DESCRIPTOR_HANDLE materialSrvGpuHandle_{};

	Triangle *triangle_ = nullptr;
	std::list<ParticleSystem::Particle> particles_;
	D3D12_GPU_DESCRIPTOR_HANDLE texHandleGPU_{};

	uint32_t currentInstanceNum_ = 0;

	AccelerationField accelerationField_{};
};

