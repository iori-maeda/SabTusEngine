#pragma once
#include <d3d12.h>
#include <vector>

enum class LayoutSemanthicType
{
	NONE,
	Position,
	Normal,
	Texcoord
};

enum class LayoutFormat
{
	FLOAT,
	FLOAT2,
	FLOAT3,
	FLOAT4
};

class DxInputLayout
{
public:
	DxInputLayout() = default;

	DxInputLayout &AddLayout(LayoutSemanthicType type, LayoutFormat format, uint32_t semanthicNum);

public:
	D3D12_INPUT_LAYOUT_DESC GetLayoutDesc() { return mLayoutDesc_; }

private:

	std::vector<D3D12_INPUT_ELEMENT_DESC> mElementDescs{};
	D3D12_INPUT_LAYOUT_DESC mLayoutDesc_{};
};

