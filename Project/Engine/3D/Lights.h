#pragma once
#include <d3d12.h>
#include <cstdint>
#include <array>
#include <vector>
#include <memory>
#include <string>

#include "Math/Vector4.h"
#include "Math/Vector3.h"
#include "ComPtr.h"

class DirectXCommon;
class Camera;

class Lights
{
public:
	static const int32_t sMaxLights = 3000;

	enum class LightType
	{
		DIRECTIONAL,
		POINT,
		SPOT,
		SUM_LIGHT_TYPE
	};

	struct Light
	{
		Vector4 color{ 1.0f, 1.0f, 1.0f,1.0f };
		Vector3 direction{ 0.0f, -1.0f, 0.0f };
		float intensity = 1.0f;
		Vector3 position{ 0.0f, 1.0f, 0.0f };
		float distance = 1.0f;
		float range = 1.0f;
		float decay = 0.0f;
		float cosFallOffStart = 0.5f;
		float cosAngle = 0.6f;

		uint32_t type = static_cast<uint32_t>(LightType::DIRECTIONAL);
	};

	struct EntryLight
	{
		uint64_t id;
		std::shared_ptr<Light> data{};
	};

public:

	void Initialize(DirectXCommon* dxCommon, Camera* camera);
	void DrawCommandSet();
	Lights::Light* AddLight(Lights::LightType type);
	void DeleteLight(uint64_t lightId);
	void AllClear();

	void DebugWindow();

public:
	uint32_t GetLightsNum() const { return useLightsNum_; }

private:

	void CreateResourceAndSRV();
	void AddLightSelect();
	void CreeateDirectionalLightWindow(Light* light);
	void CreeatePointLightWindow(Light* light);
	void CreeateSpotLightWindow(Light* light);

private:
	DirectXCommon* dxCommon_ = nullptr;
	Camera* camera_ = nullptr;

	ComPtr<ID3D12Resource> lightsResource_ = nullptr;
	Lights::Light* lightData_ = nullptr;

	D3D12_GPU_DESCRIPTOR_HANDLE lightsSrvGPUHandle_{};

	std::vector<Lights::EntryLight> lights_;
	uint32_t useLightsNum_ = 0;

	// ImGui用
	std::array<std::string, static_cast<size_t>(Lights::LightType::SUM_LIGHT_TYPE)> lightName;
};

