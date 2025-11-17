#pragma once
#include <d3d12.h>
#include <cstdint>
#include <vector>

#include "Math/Vector4.h"
#include "Math/Vector3.h"
#include "ComPtr.h"

class DirectXCommon;

class Lights
{
public:
	static const int32_t sMaxPointLights = 100;
	static const int32_t sMaxSpotLights = 100;

	struct DirectionalLight
	{
		Vector4 color{ 1.0f, 1.0f, 1.0f, 1.0f };
		Vector3 direction{ 0.0f, -1.0f, 0.0f };
		float intensity = 0.5f;
	};

	struct PointLight
	{
		Vector4 color{ 1.0f, 1.0f, 1.0f,1.0f };
		Vector3 position{ 0.0f, 1.0f, 0.0f };
		float intensity = 1.0f;
		float radius = 1.0f;
		float decay = 1.0f;
	};

	struct SpotLight
	{
		Vector4 color{ 1.0f, 1.0f, 1.0f,1.0f };
		Vector3 position{ 0.0f, 1.0f, 0.0f };
		float intensity = 1.0f;
		Vector3 direction{ 0.0f, -1.0f, 0.0f };
		float distance = 1.0f;
		float decay = 0.0f;
		float cosFallOffStart = 0.5f;
		float cosAngle = 0.6f;
	};

public:

	void Initialize(DirectXCommon* dxCommon);
	void CommandSet();
	void AddPointLight(const Lights::PointLight& pointLight) { pointLights_.push_back(pointLight); }
	void AddSpotLight(const Lights::PointLight& spotLight) { spotLights_.push_back(spotLight); }
	void AllClear();

public:
	std::vector<Lights::PointLight> GetPointLights() { pointLights_; }
	std::vector<Lights::PointLight> GetSpotLights() { spotLights_; }


private:
	DirectXCommon* dxCommon_ = nullptr;

	ComPtr<ID3D12Resource> directionalLightResource_ = nullptr;
	DirectionalLight* directionalLightData_ = nullptr;

	ComPtr<ID3D12Resource> pointLightResource_ = nullptr;
	PointLight* pointLightData_ = nullptr;

	ComPtr<ID3D12Resource> spotLightResource_ = nullptr;
	SpotLight* spotLightData_ = nullptr;

	std::vector<PointLight> pointLights_;
	std::vector<PointLight> spotLights_;
};

