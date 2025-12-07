#pragma once
#include <d3d12.h>
#include <memory>

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


public:
	DirectXCommon* GetDirectXCommon() const { return dxCommon_; }
	DxRootSignature* GetDxRootSignature() const { return dxRootSignature_.get(); }


private:

	void CreateRootSignature();
	void CreatePipelineStateObject();

private:

	DirectXCommon* dxCommon_ = nullptr;

	std::unique_ptr<DxRootSignature> dxRootSignature_ = nullptr;
	ComPtr<ID3D12PipelineState> pipelineStateObject_ = nullptr;
};

