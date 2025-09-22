#pragma once

#include <d3d12.h>

#include "ComPtr.h"

enum class BlendMode
{
	NONE,				// ブレンドしない
	ALPHA,				// 透過ブレンド Src * SrcA + Dest * (1 - SrcA)
	ADD,				// 加算ブレンド Src * SrcA + Dest * 1
	SUBTRACT,			// 減算ブレンド Src * 1 - Src * SrcA
	MULTIPLY,			// 乗算ブレンド Src * 0 + Dest * Src
	SCREEN,				// スクリーンブレンド Src * (1 - Dest) + Dest * 1
	BLEND_MODE_COUNT	// ブレンド種類数
};

namespace DirectX12ObjectsFunction
{
	ComPtr<ID3D12Resource> CreataeBufferResource(const ComPtr<ID3D12Device>&, size_t);

	D3D12_BLEND_DESC InitializeBlendMode(BlendMode blendMode);
};