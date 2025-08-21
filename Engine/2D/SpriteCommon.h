#pragma once
#include <d3d12.h>

#include "../ComPtr.h"
#include "../DirectXCommon.h"

class DirectXCommon;

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

	ComPtr<ID3D12RootSignature> rootSignature_ = nullptr;
	ComPtr<ID3D12PipelineState> pipelineStateObject_ = nullptr;
};

