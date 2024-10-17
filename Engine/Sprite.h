#pragma once

#include <d3d12.h>
#include <cstdint>

#include "ComPtr.h"
#include "./Math/Vector4.h"
#include "./Math/Vector2.h"

class DxDevice;
struct TextureData;

struct SpriteData
{
	Vector4 color{};

};

class Sprite
{
public:

	~Sprite();
	void Initialize(DxDevice *);

	void DrawSetup(const TextureData& texture);

private:

	ComPtr<ID3D12Resource> vertexResource_ = nullptr;
	ComPtr<ID3D12Resource> indexResource_ = nullptr;
	ComPtr<ID3D12Resource> colorResource_ = nullptr;
	ComPtr<ID3D12Resource> transform2DResource_ = nullptr;

	D3D12_INDEX_BUFFER_VIEW indexBufferView_{};

	struct VertexData2D
	{
		Vector4 position{};
		Vector2 uv{};
	};

	struct Transform2D
	{
		Vector2 position{};
		Vector2 scale{};
		float rotation = 0.0f;
	};

	VertexData2D *vertex_ = nullptr;
	uint32_t *indexData_ = nullptr;

public:

	Vector4 *color_ = nullptr;
	Transform2D *transform2D_ = nullptr;
};