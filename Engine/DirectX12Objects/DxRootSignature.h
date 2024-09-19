#pragma once

#include <d3d12.h>
#include <vector>
#include <map>
#include <string>

#include "../ComPtr.h"

class DxDevice;

enum class ParamType
{
	CBV = D3D12_ROOT_PARAMETER_TYPE::D3D12_ROOT_PARAMETER_TYPE_CBV,
	SRV = D3D12_ROOT_PARAMETER_TYPE::D3D12_ROOT_PARAMETER_TYPE_SRV,
	Table = D3D12_ROOT_PARAMETER_TYPE::D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
};

enum class ShaderVisibility
{
	Vertex = D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_VERTEX,
	Pixel = D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_PIXEL,
};

class DxRootSignature
{
public:
	void DefaultSettings();
	void Create(DxDevice*);

	void AddRootParameter(const std::string&,ParamType, ShaderVisibility, UINT);

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