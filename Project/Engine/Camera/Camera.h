#pragma once

#include <d3d12.h>
#include <cstdint>

#include "Math/Vector3.h"
#include "Math/Matrix4x4.h"
#include "ComPtr.h"

class Camera
{
public:
	enum class ProjectionMode : uint8_t
	{
		Perspective,
		Orthographic,

		ModeCount
	};

	struct  Transform
	{
		Vector3 scale{ 1.0f, 1.0f, 1.0f };
		Vector3 rotate{};
		Vector3 translate{};
	};

public:

	Camera() = default;
	~Camera() = default;

	void Initialize();
	void Update();
	void DebugWindow();

public:
	Vector3 GetPosition()const { return transform_.translate; }
	Matrix4x4 GetWorldMatrix() const { return worldMatrix_; }
	Matrix4x4 GetViewMatrix() const { return viewMatrix_; }
	Matrix4x4 GetProjectionMatrix() const { return projectionMatrix_; }

	void SetTransform(const Transform& transform) { transform_ = transform; }
	void SetPosition(const Vector3& position) { transform_.translate = position; }
	void SetRotation(const Vector3& rotation) { transform_.rotate = rotation; }
	void SetScale(const Vector3& scale) { transform_.scale = scale; }
	void SetFovAngleY(float fovAngleY) { fovAngleY_ = fovAngleY; }
	void SetNearZ(float nearZ) { nearZ_ = nearZ; }
	void SetFarZ(float farZ) { farZ_ = farZ; }
	void SetProjectionMode(Camera::ProjectionMode projMode) { projMode_ = projMode; }


private:

	Transform transform_{};

	float fovAngleY_ = 0.45f;
	float nearZ_ = 0.1f;
	float farZ_ = 1000.0f;

	Matrix4x4 worldMatrix_{};
	Matrix4x4 viewMatrix_{};
	Matrix4x4 projectionMatrix_{};

	ProjectionMode projMode_ = ProjectionMode::Perspective;
};