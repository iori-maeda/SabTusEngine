#pragma once
#include <d3d12.h>
#include <cstdint>
#include <vector>
#include <memory>

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

public:

	void Initialize(DirectXCommon *dxCommon);
	std::vector<Lights::Light> GetReflectLights(const Vector3 &objectPos);
	Lights::Light* AddLight(uint64_t lightType);
	void DeleteLight(uint64_t lightId);
	void AllClear();

	void DebugWindow();

public:
	uint32_t GetNumLights() { return numLights_; }

private:
	DirectXCommon *dxCommon_ = nullptr;

	std::vector<std::pair<uint64_t, std::shared_ptr<Light>>> lights_;
	uint32_t numLights_ = 0;
};

