#pragma once

#include <d3d12.h>
#include <vector>
#include <map>
#include <string>

#include "../ComPtr.h"

class DxDevice;

class DxRootSignature
{
public:
	void DefaultSettings();
	void Create(DxDevice*);

	ID3D12RootSignature* GetRootSignature();
	UINT GetParamIndex(const std::string&);
	std::vector<D3D12_ROOT_PARAMETER> GetRootParameters();
private:

	ComPtr<ID3D12RootSignature> rootSignature_ = nullptr;

	std::vector<D3D12_ROOT_PARAMETER> rootParameters_;
	std::vector<D3D12_DESCRIPTOR_RANGE> descriptorRange_;
	std::vector<D3D12_STATIC_SAMPLER_DESC> staticSamplers_;
	std::map<std::string, UINT> paramsIndexMap_;
};