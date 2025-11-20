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
	static const int32_t sMaxLights = 3000;

	enum LightType
	{
		DIRECTIONAL,
		POINT,
		SPOT
	};

	struct LightStatus
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
	void SetDrawCommand();
	uint32_t GetReflectLights(const Vector3 &objectPos);
	void AddLight(const Lights::LightStatus &light) { lights_.push_back(light); }
	void AllClear();

public:
	std::vector<Lights::LightStatus> GetLights() { lights_; }
	uint32_t GetNumLights() { return numLights_; }

private:
	void CreateSRVHandle();


private:
	DirectXCommon *dxCommon_ = nullptr;

	ComPtr<ID3D12Resource> resource_ = nullptr;
	LightStatus *lightData_ = nullptr;

	D3D12_GPU_DESCRIPTOR_HANDLE srvGPUHandle_{};

	std::vector<LightStatus> lights_;
	uint32_t numLights_ = 0;
};

