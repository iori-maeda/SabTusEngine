#pragma once
#include <d3d12.h>
#include <cstdint>
#include <string>

#include "../ComPtr.h"
#include "../Math/Vector4.h"
#include "../Math/Vector3.h"
#include "../Math/Vector2.h"
#include "../Math/Matrix4x4.h"

class SpriteRenderer;

class Sprite
{
public:

	struct VertexData
	{
		Vector4 position{};
		Vector2 uv{};
	};

	struct MaterialData
	{
		Vector4 color{};
	};

	struct TransformationMatrix
	{
		Matrix4x4 wvp{};
		Matrix4x4 world{};
	};

public:

	Sprite() = default;

	void Initiazlize(SpriteRenderer* renderer, const std::string& fileName);
	void Upadate();
	void Draw();

public:

	void SetPosition(const Vector2& position) { position_ = position; }
	void SetRotation(float rotation) { rotation_ = rotation; }
	void SetScale(const Vector2& scale) { scale_ = scale; }
	void SetColor(const Vector4& color) { color_ = color; }
	void SetAnchorPoiont(const Vector2& anchorPoint) { anchorPoint_ = anchorPoint; }

private:

	SpriteRenderer* renderer_ = nullptr;

	Vector2 position_{};
	float rotation_ = 0.0f;
	Vector2 scale_{ 1.0f, 1.0f };
	Vector4 color_{ 1.0f, 1.0f, 1.0f, 1.0f };
	Vector2 anchorPoint_{};
	Vector2 size_{};

	ComPtr<ID3D12Resource> vertexResource_ = nullptr;
	ComPtr<ID3D12Resource> indexResource_ = nullptr;
	ComPtr<ID3D12Resource> materialResource_ = nullptr;
	ComPtr<ID3D12Resource> transformationMatrixResource_ = nullptr;

	VertexData* vertexData_ = nullptr;
	uint32_t* indexData_ = nullptr;
	MaterialData* materialData_ = nullptr;
	TransformationMatrix* transformationMatrixData_ = nullptr;

	D3D12_GPU_DESCRIPTOR_HANDLE texHandle_{};
	Vector2 texSize_{};


	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};
	D3D12_INDEX_BUFFER_VIEW indexBufferView_{};

};