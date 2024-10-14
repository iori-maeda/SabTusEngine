#pragma once

#include <d3d12.h>
#include <vector>
#include <map>
#include <string>

#include "../ComPtr.h"

class DxDevice;

enum class ParamType
{
	PixelCBuffer,
	PixelTex,
	VertexCbuffer,
	VertexTex
};

class DxRootSignature
{
public:
	void DefaultSettings();
	void Create(DxDevice*);

	void AddRootParameter(ParamType type, UINT numDescriptors, UINT startReginsterIndex);

	ID3D12RootSignature* GetRootSignature();
	std::vector<D3D12_ROOT_PARAMETER> GetRootParameters();
private:

	ComPtr<ID3D12RootSignature> rootSignature_ = nullptr;

	// texture用
	std::vector<D3D12_DESCRIPTOR_RANGE> descriptorRange_;
	std::vector<D3D12_STATIC_SAMPLER_DESC> staticSamplers_;

	// パラメタコンテナ
	std::vector<D3D12_ROOT_PARAMETER> rootParameters_;
	UINT paramIndex_ = 0;
};