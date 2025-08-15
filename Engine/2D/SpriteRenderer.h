#pragma once
#include <d3d12.h>

#include "../ComPtr.h"
#include "../DxRenderContext.h"

class DxRenderContext;

class SpriteRenderer
{
public:
	SpriteRenderer() = default;

	void Initialize(DxRenderContext* renderContext);
	void PreDraw();

	DxRenderContext* GetRenderContext() const { return renderContext_; }

private:
	
	void CreateRootSignature();
	void CreatePipelineStateObject();

private:

	DxRenderContext* renderContext_ = nullptr;

	ComPtr<ID3D12RootSignature> rootSignature_ = nullptr;
	ComPtr<ID3D12PipelineState> pipelineStateObject_ = nullptr;
};

