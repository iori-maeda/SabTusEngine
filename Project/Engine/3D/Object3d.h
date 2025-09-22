#pragma once
#include <d3d12.h>
#include <cstdint>
#include <string>
#include <memory> 

#include "ComPtr.h"
#include "Math/Vector4.h"
#include "Math/Vector3.h"
#include "Math/Vector2.h"
#include "Math/Matrix4x4.h"
#include "Model.h"

class Object3dCommon;
class Camera;

class Object3d
{
public:

	struct  Transform
	{
		Vector3 scale{ 1.0f, 1.0f, 1.0f };
		Vector3 rotate{};
		Vector3 translate{};
	};

	struct TransformationMatrix
	{
		Matrix4x4 wvp{};
		Matrix4x4 world{};
	};

public:

	Object3d() = default;
	~Object3d();

	void Initiazlize(Object3dCommon *renderer, const std::string &fileName);
	void Upadate();
	void Draw();

	void DebugWindow();

public:

	std::string GetModelName() const { return model_->GetName(); }
	Vector3 GetPosition() const { return transform_.translate; }

	void SetTransform(const Transform &transform) { transform_ = transform; }
	void SetPosition(const Vector3 &position) { transform_.translate = position; }
	void SetRotation(const Vector3 &rotation) { transform_.rotate = rotation; }
	void SetScale(const Vector3 &scale) { transform_.scale = scale; }
	void SetColor(const Vector4 &color)
	{
		if (model_)
		{
			model_->SetColor(color);
		}
	}
	void SetCamera(Camera *camera) { camera_ = camera; }


private:

	Object3dCommon *obj3dCommon_ = nullptr;
	Camera *camera_ = nullptr;

	Transform transform_{};

	ComPtr<ID3D12Resource> transformationMatrixResource_ = nullptr;
	TransformationMatrix *transformationMatrixData_ = nullptr;

	Model *model_ = nullptr;
};

