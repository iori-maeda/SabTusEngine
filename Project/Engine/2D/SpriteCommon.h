#pragma once
#include <d3d12.h>

#include "ComPtr.h"
#include "DirectXCommon.h"

class DirectXCommon;
class DxRootSignature;

class SpriteCommon
{
public:
	SpriteCommon() = default;

	void Initialize(DirectXCommon* dxCommon);
	void PreDraw();

	DirectXCommon* GetDirectXCommon() const { return dxCommon_; }

private:
	
	void CreateRootSignature();
	void CreatePipelineStateObject();

private:

	DirectXCommon* dxCommon_ = nullptr;

	ComPtr<DxRootSignature> dxRootSignature_ = nullptr;
	ComPtr<ID3D12PipelineState> pipelineStateObject_ = nullptr;
};

