#pragma once

#include <d3d12.h>
#include <cstdint>

#include "ComPtr.h"
#include "./Math/Vector4.h"
#include "./Math/Vector2.h"

class DxDevice;

class Sprite
{
public:

	~Sprite();
	void Initialize(DxDevice *);

private:

	ComPtr<ID3D12Resource> vertexResource_ = nullptr;
	ComPtr<ID3D12Resource> indexResource_ = nullptr;
	ComPtr<ID3D12Resource> colorResource_ = nullptr;

	D3D12_INDEX_BUFFER_VIEW indexBufferView_{};

	struct VertexData2D
	{
		Vector4 position{};
		Vector2 uv{};
	};

	VertexData2D *vertexData_ = nullptr;
	Vector4 *colorData_ = nullptr;
	uint32_t *indexData_ = nullptr;
};