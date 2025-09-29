#pragma once
#include <d3d12.h>

#include "ComPtr.h"
#include "Math/Vector2.h"
#include "Math/Vector3.h"
#include "Math/Vector4.h"
#include "Math/Matrix4x4.h"
#include "Camera/Camera.h"

class DirectXCommon;

class Triangle
{
public:

	struct  Transform
	{
		Vector3 scale{ 1.0f, 1.0f, 1.0f };
		Vector3 rotate{};
		Vector3 translate{};
	};

	struct VertexData
	{
		Vector4 position{};
		Vector2 uv{};
	};

public:
	Triangle() = default;
	~Triangle();

	void Initialize(DirectXCommon *dxCommon, const Vector3 &position = Vector3());
	void Update();
	void Draw();

public:
	uint32_t GetVerticiesNum() { return 3; }
	D3D12_VERTEX_BUFFER_VIEW GetVertexbufferView() { return vertexBufferView_; }
	Transform GetTransform() { return transform_; }

	void SetCamera(Camera *camera) { camera_ = camera; }
	void SetPosition(const Vector3 &position) { transform_.translate = position; }
	void SetRotate(const Vector3 &rotation) { transform_.rotate = rotation; }
	void SetScale(const Vector3 &scale) { transform_.scale = scale; }
	void SetTransform(const Transform &transform) { transform_ = transform; }

private:

	DirectXCommon *dxCommon_ = nullptr;
	Camera *camera_ = nullptr;

	Transform transform_{};

	ComPtr<ID3D12Resource> vertexResource_ = nullptr;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};
	VertexData *vertexData_ = nullptr;
};

