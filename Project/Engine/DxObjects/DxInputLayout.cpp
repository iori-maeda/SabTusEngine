#include "DxInputLayout.h"

DxInputLayout &DxInputLayout::AddLayout(LayoutSemanthicType type, LayoutFormat format, uint32_t semanthicIndex)
{
    D3D12_INPUT_ELEMENT_DESC newDesc{};

	switch (type)
	{
	case LayoutSemanthicType::NONE:
		break;
	case LayoutSemanthicType::Position:
		newDesc.SemanticName = "POSITION";
		break;
	case LayoutSemanthicType::Normal:
		newDesc.SemanticName = "NORMAL";
		break;
	case LayoutSemanthicType::Texcoord:
		newDesc.SemanticName = "TEXCOORD";
		break;
	case LayoutSemanthicType::Tangernt:
		newDesc.SemanticName = "TANGENT";
		break;
	default:
		break;
	}

	switch (format)
	{
	case LayoutFormat::FLOAT:
		newDesc.Format = DXGI_FORMAT_R32_FLOAT;
		break;
	case LayoutFormat::FLOAT2:
		newDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
		break;
	case LayoutFormat::FLOAT3:
		newDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
		break;
	case LayoutFormat::FLOAT4:
		newDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		break;
	default:
		break;
	}

	newDesc.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	newDesc.SemanticIndex = semanthicIndex;

	mElementDescs.push_back(newDesc);

	mLayoutDesc_.pInputElementDescs = mElementDescs.data();
    mLayoutDesc_.NumElements = static_cast<UINT>(mElementDescs.size());
	
    return *this;
}