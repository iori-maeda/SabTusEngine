#pragma once
#include <vector>
#include <unordered_map>
#include <d3d12.h>
#include <cstdint>
#include <memory>

#include "ComPtr.h"

class DxRootSignature
{
public:
	enum class ParamSemanticType
	{
		TransformationMatrix,
		CameraTransform,
		Texture,
		Lights,
		ObjectMaterial,
		MeshMaterial,
		Particle,
		Essential
	};

	enum class ParamType
	{
		CBV,
		SRV,
		UAV,
		DescriptorTable
	};

	enum class ShaderVisibility
	{
		Vertex,
		Pixel,
		All
	};

	struct RootParamSemantic
	{
		ParamSemanticType semanticType;
		D3D12_GPU_VIRTUAL_ADDRESS* gpuAddressPtr = nullptr;
		D3D12_GPU_DESCRIPTOR_HANDLE* gpuDescriptorHandlePtr = nullptr;
	};

public:
	DxRootSignature() = default;

	void Initialize(ID3D12Device* device);
	/// <summary>
	/// ルートパラメータの追加とセマンティクスの登録
	/// </summary>
	/// <param name="semanticType">追加するセマンティクス</param>
	/// <param name="paramType">パラメータのタイプ</param>
	/// <param name="shaderVisiblity">どのシェーダからアクセスさせるか</param>
	/// <param name="regster">使用レジスタ</param>
	/// <param name="numDescriptors">確保数</param>
	/// DescriptorTableを選択時はregsterをBaseShaderRegister、numDescriptorsをNumDescriptorsとして使用される
	void AddRootParamSemantic(ParamSemanticType semanticType, ParamType paramType, ShaderVisibility shaderVisiblity, UINT regster = 0, UINT numDescriptors = 0);

public:
	UINT GetRootParamIndex(ParamSemanticType type) const { return rootParamSemantics_.at(type); }

public:
	ID3D12RootSignature* GetRootSignature() const { return rootSignature_.Get(); }

private:
	ComPtr<ID3D12RootSignature> rootSignature_;

	std::vector<D3D12_ROOT_PARAMETER> rootParameters_;
	std::unordered_map<ParamSemanticType, UINT> rootParamSemantics_;
	std::vector<std::unique_ptr<D3D12_DESCRIPTOR_RANGE>> descriptorRanges_;
};

