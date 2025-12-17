#include "DxObjFunctions.h"

#include <cassert>
#include <format>

#include "Logger.h"

namespace DxObjFunctions
{
	/// <summary>
	/// リソース作成関数
	/// </summary>
	/// <param name="device->GetDevice()">作成してくれるデバイス</param>
	/// <param name="sizeInBytes">使用サイズ</param>
	/// <returns></returns>
	ComPtr<ID3D12Resource> CreataeBufferResource(const ComPtr<ID3D12Device> &device, size_t sizeInBytes)
	{
		D3D12_HEAP_PROPERTIES uploadHeapPoperties{};
		uploadHeapPoperties.Type = D3D12_HEAP_TYPE_UPLOAD;
		D3D12_RESOURCE_DESC resourceDesc{};
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		resourceDesc.Width = sizeInBytes;
		/// バッファのここはテンプレ
		resourceDesc.Height = 1;
		resourceDesc.DepthOrArraySize = 1;
		resourceDesc.MipLevels = 1;
		resourceDesc.SampleDesc.Count = 1;
		resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		/// ここまでテンプレ
		ComPtr<ID3D12Resource> resource = nullptr;
		HRESULT hr = device->CreateCommittedResource(&uploadHeapPoperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&resource));
		assert(SUCCEEDED(hr));
		Logger::Log("Created Resource\n");
		return resource;
	}

	D3D12_BLEND_DESC InitializeBlendMode(BlendMode blendMode)
	{
		D3D12_BLEND_DESC blendDesc{};
		blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

		switch (blendMode)
		{
		case BlendMode::NONE:
			blendDesc.RenderTarget[0].BlendEnable = false;
			break;

		case BlendMode::ALPHA:
			blendDesc.RenderTarget[0].BlendEnable = true;
			blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
			blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
			blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
			blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
			blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
			blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
			break;

		case BlendMode::ADD:
			blendDesc.RenderTarget[0].BlendEnable = true;
			blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
			blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
			blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
			blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
			blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
			blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
			break;

		case BlendMode::SUBTRACT:
			blendDesc.RenderTarget[0].BlendEnable = true;
			blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
			blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
			blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_REV_SUBTRACT;
			blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
			blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
			blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
			break;

		case BlendMode::MULTIPLY:
			blendDesc.RenderTarget[0].BlendEnable = true;
			blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ZERO;
			blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_SRC_COLOR;
			blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
			blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
			blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
			blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
			break;

		case BlendMode::SCREEN:
			blendDesc.RenderTarget[0].BlendEnable = true;
			blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_INV_DEST_COLOR;
			blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
			blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
			blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
			blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
			blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
			break;

		default:
			Logger::Log(std::format("Not Find to BlendMode : {}", static_cast<int>(blendMode)));
			break;
		}

		return blendDesc;
	}
	D3D12_RASTERIZER_DESC InitializeRasterizerState(CullingMode cullingMode, bool isFillModeSolid)
	{
		D3D12_RASTERIZER_DESC rasterizerDesc{};
		switch (cullingMode)
		{
		case CullingMode::None:
			rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
			break;
		case CullingMode::Back:
			rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
			break;
		case CullingMode::Front:
			rasterizerDesc.CullMode = D3D12_CULL_MODE_FRONT;
			break;
		default:
			break;
		}


		if (isFillModeSolid)
		{
			rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
		}
		else
		{
			rasterizerDesc.FillMode = D3D12_FILL_MODE_WIREFRAME;
		}
		
		return rasterizerDesc;
	}
	D3D12_DEPTH_STENCIL_DESC InitializeDepthStencilState(DepthMode depthMode, bool isDepthTestEnable, bool isDepthWriteEnable)
	{
		D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};

		depthStencilDesc.DepthEnable = isDepthTestEnable;
		depthStencilDesc.DepthWriteMask = isDepthWriteEnable ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;

		switch (depthMode)
		{
		case DepthMode::None:
			depthStencilDesc = {};
			depthStencilDesc.DepthEnable = false;
			break;
		case DepthMode::Less:
			depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
			break;
		case DepthMode::LessEqual:
			depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
			break;
		case DepthMode::Equal:
			depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_EQUAL;
			break;
		case DepthMode::GreaterEqual:
			depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_GREATER_EQUAL;
			break;
		case DepthMode::Greater:
			depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_GREATER;
			break;
		default:
			break;
		}

		return depthStencilDesc;
	}
}