#pragma once
#include <d3d12.h>
#include <memory>

#include "ComPtr.h"
#include "DirectXCommon.h"
#include "DxRootSignature.h"
#include "DxPipelineStateObject.h"
#include "Math/Vector4.h"
#include "Math/Vector3.h"

class DirectXCommon;
class Lights;

class Object3dCommon
{
public:
	struct Essential
	{
		uint32_t numLights = 0;
		int drawPBR = 0;
	};

	struct FogStatus
	{
		float density = 0.05f;
		float power = 1.5f;
		float thresholdStart = 0.3f;
		float thresholdEnd = 0.1f;
	};

public:

	Object3dCommon() = default;
	~Object3dCommon();

	void Initialize(DirectXCommon *dxCommon);
	void PreDraw();

	void DebugWindow();

public:
	DirectXCommon *GetDirectXCommon() const { return dxCommon_; }
	DxRootSignature *GetDxRootSignature() const { return dxRootSignature_.get(); }

	void SetLights(Lights *lights) { lights_ = lights; }

private:

	void CreateRootSignature();
	void CreatePipelineStateObject();

private:

	DirectXCommon *dxCommon_ = nullptr;
	Lights *lights_ = nullptr;

	std::unique_ptr<DxRootSignature> dxRootSignature_ = nullptr;
	//ComPtr<ID3D12PipelineState> pipelineStateObject_ = nullptr;
	std::unique_ptr<DxPipelineStateObject> dxPipelineStateObject_ = nullptr;

	ComPtr<ID3D12Resource> essentialResource_ = nullptr;
	Essential *essentialForGPUData_ = nullptr;

	ComPtr<ID3D12Resource> fogResource_ = nullptr;
	FogStatus *fogData_ = nullptr;

	
};

