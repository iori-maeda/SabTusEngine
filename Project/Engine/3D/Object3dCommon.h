#pragma once
#include <d3d12.h>

#include "ComPtr.h"
#include "DirectXCommon.h"
#include "Math/Vector4.h"
#include "Math/Vector3.h"

class DirectXCommon;
class Lights;

class Object3dCommon
{
public:

	Object3dCommon() = default;
	~Object3dCommon();

	void Initialize(DirectXCommon* dxCommon);
	void PreDraw();

	void DebugWindow();

public:
	DirectXCommon* GetDirectXCommon() const { return dxCommon_; }
	Lights* GetLights() const { return lights_; }

	void SetLights(Lights* lights) { lights_ = lights; }

private:

	void CreateRootSignature();
	void CreatePipelineStateObject();

private:

	DirectXCommon* dxCommon_ = nullptr;

	ComPtr<ID3D12RootSignature> rootSignature_ = nullptr;
	ComPtr<ID3D12PipelineState> pipelineStateObject_ = nullptr;

	Lights* lights_ = nullptr;
};

