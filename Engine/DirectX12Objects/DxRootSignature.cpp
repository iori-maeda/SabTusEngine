#include "DxRootSignature.h"

#include <cassert>
#include <format>

#include "DxDevice.h"
#include "DxCommand.h"
#include "../Logger.h"

//void DxRootSignature::DefaultSettings()
//{
//	CreateDescriptorRange();
//
//#pragma region RootParameter Create
//
//	rootParameters_[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
//	rootParameters_[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
//	rootParameters_[0].Descriptor.ShaderRegister = 0;
//
//	rootParameters_[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
//	rootParameters_[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
//	rootParameters_[1].Descriptor.ShaderRegister = 0;
//
//	rootParameters_[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
//	rootParameters_[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
//	rootParameters_[2].DescriptorTable.pDescriptorRanges = descriptorRange_.data();
//	rootParameters_[2].DescriptorTable.NumDescriptorRanges = static_cast<UINT>(descriptorRange_.size());
//
//	rootParameters_[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
//	rootParameters_[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
//	rootParameters_[3].Descriptor.ShaderRegister = 1;
//#pragma endregion
//#pragma region Smapler Settings
//	staticSamplers_.resize(1);
//
//	staticSamplers_[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;		// バイナリフィルタ
//	staticSamplers_[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;		// 0~1リピート
//	staticSamplers_[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;		// 
//	staticSamplers_[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;		// 
//	staticSamplers_[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;	// 比較しない
//	staticSamplers_[0].MaxLOD = D3D12_FLOAT32_MAX;						// 最大まで使用
//	staticSamplers_[0].ShaderRegister = 0;
//	staticSamplers_[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
//#pragma endregion
//}

void DxRootSignature::Create(DxDevice* device)
{
	CreateStaticSamplers();

	D3D12_ROOT_SIGNATURE_DESC descriptorRootSignature{};
	descriptorRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	descriptorRootSignature.pParameters = params_.data();
	descriptorRootSignature.NumParameters = static_cast<UINT>(rootParameters_.size());
	descriptorRootSignature.pStaticSamplers = staticSamplers_.data();
	descriptorRootSignature.NumStaticSamplers = static_cast<UINT>(staticSamplers_.size());
	// シリアライズしてバイナリ化
	ComPtr<ID3DBlob> signatureBlob = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&descriptorRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED(hr))
	{
		Logger::Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);
	}
	// バイナリを基に生成
	hr = device->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature_));
	assert(SUCCEEDED(hr));
	Logger::Log("Created RootSignature\n");
}

void DxRootSignature::AddRootParameter(const std::string& key, const Dx12Structs::RootParamMaterials& addParamMaterials)
{
	ParamData addParam{};
	//D3D12_DESCRIPTOR_RANGE descriptorRange{};
	addParam.key = key;
	switch (addParamMaterials.type)
	{
	case ParamType::PixelCBuffer:
		addParam.param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		addParam.param.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		addParam.param.Descriptor.ShaderRegister = static_cast<UINT>(addParamMaterials.useRegister);
		break;

	case ParamType::PixelTex:
		CreateDescriptorRange();

		addParam.param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		addParam.param.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		addParam.param.DescriptorTable.pDescriptorRanges = descriptorRange_.data();
		addParam.param.DescriptorTable.NumDescriptorRanges = static_cast<UINT>(descriptorRange_.size());
		break;

	case ParamType::VertexCbuffer:
		addParam.param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		addParam.param.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		addParam.param.Descriptor.ShaderRegister = static_cast<UINT>(addParamMaterials.useRegister);
		break;

	case ParamType::VertexTex:
		addParam.param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		addParam.param.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		addParam.param.Descriptor.ShaderRegister = static_cast<UINT>(addParamMaterials.useRegister);
		break;

	default:
		Logger::Log("Not Found Type\n");
		return;
		//break;
	}

	// コマンドに積む用のコンテナに追加
	rootParameters_.push_back(addParam);
	// rootSignature作成用コンテナにも追加
	// 微妙の気配がぬぐえないので暇が出来たら必ず修正
	params_.push_back(addParam.param);

	if (addParamMaterials.type == ParamType::PixelTex)
	{
		texIndex = static_cast<int>(rootParameters_.size() - 1);
	}
}

void DxRootSignature::CreateDescriptorRange()
{
	if (!descriptorRange_.empty())
	{
		return;
	}
	else
	{
		descriptorRange_.resize(1);
		descriptorRange_[0].BaseShaderRegister = 0;
		descriptorRange_[0].NumDescriptors = 1;

		descriptorRange_[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		descriptorRange_[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	}
}

void DxRootSignature::CreateStaticSamplers()
{
	staticSamplers_.resize(1);

	staticSamplers_[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;		// バイナリフィルタ
	staticSamplers_[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;		// 0~1リピート
	staticSamplers_[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;		// 
	staticSamplers_[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;		// 
	staticSamplers_[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;	// 比較しない
	staticSamplers_[0].MaxLOD = D3D12_FLOAT32_MAX;						// 最大まで使用
	staticSamplers_[0].ShaderRegister = 0;
	staticSamplers_[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
}

void DxRootSignature::SetDescriptorRange(UINT baseRegsterIndex = 0, UINT numIRegisters = 1)
{
	CreateDescriptorRange();
	descriptorRange_[0].BaseShaderRegister = baseRegsterIndex;
	descriptorRange_[0].NumDescriptors = numIRegisters;

	descriptorRange_[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange_[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
}

ID3D12RootSignature* DxRootSignature::GetRootSignature()
{
	return rootSignature_.Get();
}

void DxRootSignature::SetCommands(DxCommand* command, D3D12_GPU_DESCRIPTOR_HANDLE handle)
{
	assert(command != nullptr);
	if (command == nullptr) { return; }
	for (int i = 0; i < static_cast<int>(rootParameters_.size()); ++i)
	{
		if (i == texIndex)
		{
			command->GetCommandList()->SetGraphicsRootDescriptorTable(static_cast<UINT>(i), handle);
		}
		else
		{
			command->GetCommandList()->SetGraphicsRootConstantBufferView(static_cast<UINT>(i), rootParameters_[i].resource->GetGPUVirtualAddress());
		}
	}
}