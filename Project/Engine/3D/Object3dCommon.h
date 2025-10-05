#pragma once
#include <d3d12.h>

#include "ComPtr.h"
#include "DirectXCommon.h"
#include "Math/Vector4.h"
#include "Math/Vector3.h"

class DirectXCommon;

class Object3dCommon
{
public:

	Object3dCommon() = default;
	~Object3dCommon();

	void Initialize(DirectXCommon *dxCommon);
	void PreDraw();

	void DebugWindow();

	DirectXCommon *GetDirectXCommon() const { return dxCommon_; }

private:

	void CreateRootSignature();
	void CreatePipelineStateObject();

private:

	DirectXCommon *dxCommon_ = nullptr;

	ComPtr<ID3D12RootSignature> rootSignature_ = nullptr;
	ComPtr<ID3D12PipelineState> pipelineStateObject_ = nullptr;

	struct DirectionalLight
	{
		Vector4 color{};
		Vector3 direction{};
		float intensity = 0.0f;
	};

	struct PointLight
	{
		Vector4 color{ 1.0f, 1.0f, 1.0f,1.0f };
		Vector3 position{};
		float intensity = 1.0f;
		float radius = 1.0f;
		float decay = 1.0f;
		float pad[2]{};
	};

	struct SpotLight
	{
		Vector4 color{};
		Vector3 position{};
		float intensity = 0.0f;
		Vector3 direction{};
		float distance = 0.0f;
		float decay = 0.0f;
		float cosFallOffStart = 0.0f;
		float cosAngle = 0.0f;
		float pad[2]{};
	};

	ComPtr<ID3D12Resource> directionalLightResource_ = nullptr;
	DirectionalLight *directionalLightData_ = nullptr;

	ComPtr<ID3D12Resource> pointLightResource_ = nullptr;
	PointLight *pointLightData_ = nullptr;

	ComPtr<ID3D12Resource> spotLightResource_ = nullptr;
	SpotLight* spotLightData_ = nullptr;
};

