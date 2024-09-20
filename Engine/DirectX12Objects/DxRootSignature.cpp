#include "DxRootSignature.h"

#include <cassert>

#include "DxDevice.h"
#include "../Logger.h"

void DxRootSignature::DefaultSettings()
{
	descriptorRange_.resize(1);

	// Texture用
	descriptorRange_[0].BaseShaderRegister = 0;
	descriptorRange_[0].NumDescriptors = 1;
	descriptorRange_[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange_[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

#pragma region RootParameter Create
	rootParameters_.resize(4);

	rootParameters_[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters_[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters_[0].Descriptor.ShaderRegister = 0;

	rootParameters_[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters_[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameters_[1].Descriptor.ShaderRegister = 0;

	rootParameters_[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters_[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters_[2].DescriptorTable.pDescriptorRanges = descriptorRange_.data();
	rootParameters_[2].DescriptorTable.NumDescriptorRanges = static_cast<UINT>(descriptorRange_.size());

	rootParameters_[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters_[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters_[3].Descriptor.ShaderRegister = 1;
#pragma endregion
#pragma region Smapler Settings
	staticSamplers_.resize(1);

	staticSamplers_[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;		// バイナリフィルタ
	staticSamplers_[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;		// 0~1リピート
	staticSamplers_[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;		// 
	staticSamplers_[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;		// 
	staticSamplers_[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;	// 比較しない
	staticSamplers_[0].MaxLOD = D3D12_FLOAT32_MAX;						// 最大まで使用
	staticSamplers_[0].ShaderRegister = 0;
	staticSamplers_[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
#pragma endregion
}

void DxRootSignature::Create(DxDevice *device)
{
	D3D12_ROOT_SIGNATURE_DESC descriptorRootSignature{};
	descriptorRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	descriptorRootSignature.pParameters = rootParameters_.data();
	descriptorRootSignature.NumParameters = static_cast<UINT>(rootParameters_.size());
	descriptorRootSignature.pStaticSamplers = staticSamplers_.data();
	descriptorRootSignature.NumStaticSamplers = static_cast<UINT>(staticSamplers_.size());
	// シリアライズしてバイナリ化
	ComPtr<ID3DBlob> signatureBlob = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&descriptorRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED(hr))
	{
		Logger::Log(reinterpret_cast<char *>(errorBlob->GetBufferPointer()));
		assert(false);
	}
	// バイナリを基に生成
	hr = device->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature_));
	assert(SUCCEEDED(hr));
	Logger::Log("Created RootSignature\n");
}

void DxRootSignature::AddRootParameter(ParamType type, ID3D12Resource *setResource)
{
	D3D12_ROOT_PARAMETER addParam{};
	switch (type)
	{
	case ParamType::PixelCBuffer:
		rootParameters_[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		rootParameters_[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		rootParameters_[0].Descriptor.ShaderRegister = 0;
		break;
	case ParamType::PixelTex:
		descriptorRange_[0].BaseShaderRegister = 0;
		descriptorRange_[0].NumDescriptors = 1;
		descriptorRange_[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		descriptorRange_[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		rootParameters_[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootParameters_[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		rootParameters_[2].DescriptorTable.pDescriptorRanges = descriptorRange_.data();
		rootParameters_[2].DescriptorTable.NumDescriptorRanges = static_cast<UINT>(descriptorRange_.size());
		break;
	case ParamType::VertexCbuffer:
		rootParameters_[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		rootParameters_[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		rootParameters_[0].Descriptor.ShaderRegister = 0;
		break;
	case ParamType::VertexTex:
		rootParameters_[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		rootParameters_[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		rootParameters_[0].Descriptor.ShaderRegister = 0;
		break;
	default:
		break;
	}
}

ID3D12RootSignature *DxRootSignature::GetRootSignature()
{
	return rootSignature_.Get();
}

std::vector<D3D12_ROOT_PARAMETER> DxRootSignature::GetRootParameters()
{
	return rootParameters_;
}