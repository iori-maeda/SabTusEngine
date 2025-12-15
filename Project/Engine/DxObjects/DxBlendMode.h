#pragma once
#include <d3d12.h>
#include <vector>

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

class DxBlendMode
{
public:
	DxBlendMode() = default;

	DxBlendMode &AddUseMode(BlendMode blendMode);
	std::vector<D3D12_BLEND_DESC> &GetBlendModeDescs() { return modes_; }

private:
	std::vector<D3D12_BLEND_DESC> modes_;
};