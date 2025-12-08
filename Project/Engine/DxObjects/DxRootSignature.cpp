#include "DxRootSignature.h"
#include <cassert>

#include "Logger.h"


void DxRootSignature::Create(ID3D12Device *device)
{
	D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;			// バイナリフィルタ
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;		// 0~1リピート
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;		// 
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;		// 
	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;		// 比較しない
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;						// 最大まで使用
	staticSamplers[0].ShaderRegister = 0;
	staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

#pragma region RootSignature Create
	D3D12_ROOT_SIGNATURE_DESC descriptorRootSignature{};
	descriptorRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	descriptorRootSignature.pParameters = rootParameters_.data();
	descriptorRootSignature.NumParameters = static_cast<UINT>(rootParameters_.size());
	descriptorRootSignature.pStaticSamplers = staticSamplers;
	descriptorRootSignature.NumStaticSamplers = _countof(staticSamplers);
	// シリアライズしてバイナリ化
	ComPtr<ID3DBlob> signatureBlob = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&descriptorRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED(hr))
	{
		Logger::Log(reinterpret_cast<char *>(errorBlob->GetBufferPointer()));
		assert(false);
	}
	hr = device->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature_));
	assert(SUCCEEDED(hr));
	Logger::Log("Created RootSignature\n");
#pragma endregion
}

void DxRootSignature::Initialize(ID3D12Device *device, RootParamInfo *rootParams, UINT numRootParams)
{
	rootParameters_.reserve(numRootParams);
	for (UINT i = 0; i < numRootParams; ++i)
	{
		AddRootParamSemantic(
			rootParams[i].semanticType,
			rootParams[i].type,
			rootParams[i].shaderVisibility,
			rootParams[i].registerIndex,
			rootParams[i].numDescriptors
		);
	}
	Create(device);
}

DxRootSignature& DxRootSignature::AddRootParamSemantic(ParamSemanticType semanticType, ParamType paramType, ShaderVisibility shaderVisiblity, UINT useRegister, UINT numDescriptors)
{
	D3D12_ROOT_PARAMETER addParam{};

	switch (paramType)
	{
	case ParamType::CBV:
		addParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		break;
	case ParamType::SRV:
		addParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
		break;
	case ParamType::UAV:
		addParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
		break;
	case ParamType::DescriptorTable:
		addParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		{
			std::unique_ptr<D3D12_DESCRIPTOR_RANGE> descriptorRange = std::make_unique<D3D12_DESCRIPTOR_RANGE>();

			descriptorRange->BaseShaderRegister = useRegister;
			descriptorRange->NumDescriptors = numDescriptors;
			descriptorRange->RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
			descriptorRange->OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

			addParam.DescriptorTable.pDescriptorRanges = descriptorRange.get();
			addParam.DescriptorTable.NumDescriptorRanges = 1;

			descriptorRanges_.push_back(std::move(descriptorRange));
		}
		break;
	default:

		break;
	}

	switch (shaderVisiblity)
	{
	case ShaderVisibility::Vertex:
		addParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
		break;
	case ShaderVisibility::Pixel:
		addParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		break;
	case ShaderVisibility::All:
		addParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		break;
	default:
		break;
	}

	if (paramType != ParamType::DescriptorTable)
	{
		addParam.Descriptor.ShaderRegister = useRegister;
	}
	rootParameters_.push_back(addParam);
	rootParamSemantics_.emplace(semanticType, static_cast<UINT>(rootParameters_.size() - 1));

	return *this;
}

