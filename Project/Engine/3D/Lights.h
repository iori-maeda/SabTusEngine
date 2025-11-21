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

class Lights
{
public:
	static const int32_t sMaxLights = 3000;

	enum LightType
	{
		DIRECTIONAL,
		POINT,
		SPOT,
		SUM_LIGHT_TYPE
	};

	struct Light
	{
		uint32_t type = DIRECTIONAL;
		Vector3 direction{ 0.0f, -1.0f, 0.0f };
		Vector4 color{ 1.0f, 1.0f, 1.0f,1.0f };
		float intensity = 1.0f;
		Vector3 position{ 0.0f, 1.0f, 0.0f };
		float distance = 1.0f;
		float range = 1.0f;
		float decay = 0.0f;
		float cosFallOffStart = 0.5f;
		float cosAngle = 0.6f;
	};

	struct EntryLight
	{
		uint64_t id;
		std::shared_ptr<Light> data{};
	};

public:

	void Initialize(DirectXCommon* dxCommon);
	std::vector<Lights::Light> GetReflectLights(const Vector3& objectPos);
	Lights::Light* AddLight(uint64_t lightType);
	void DeleteLight(uint64_t lightId);
	void AllClear();

	void DebugWindow();

private:
	void AddLightSelect();
	void CreeateDirectionalLightWindow(Light* light);
	void CreeatePointLightWindow(Light* light);
	void CreeateSpotLightWindow(Light* light);

private:
	DirectXCommon* dxCommon_ = nullptr;

	std::vector<Lights::EntryLight> lights_;

	// ImGui用
	std::array<std::string, SUM_LIGHT_TYPE> lightName;
};

