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
#include "Lights.h"

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
		Matrix4x4 worldInverseTranspose{};
	};

	struct CameraForGPU
	{
		Vector3 worldPosition;
		float pad1;
		Matrix4x4 viewMat{};
	};

	struct ObjectMaterial
	{
		Vector4 color{ 1.0f ,1.0f, 1.0f, 1.0f };
		float metallic = 0.0f;
		float roughness = 0.0f;
		uint32_t enableLighting;
		uint32_t useTexture;
		uint32_t caluclatePBR;
		uint32_t useNormal;
	};


public:

	Object3d() = default;
	~Object3d();

	void Initialize(Object3dCommon *obj3dCommon, const std::string &fileName);
	void Initialize(Object3dCommon *obj3dCommon, Model *model);
	void Upadate();
	void Draw();

	void DebugWindow();

public:

	std::string GetModelName() const { return model_->GetName(); }
	Vector3 GetPosition() const { return transform_.translate; }

	void SetTransform(const Model::Transform &transform) { transform_ = transform; }
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

	void CreateResource();
	void DrawNode(const Model::Node &node);

private:

	Object3dCommon *obj3dCommon_ = nullptr;
	Camera *camera_ = nullptr;

	Model::Transform transform_{};

	ComPtr<ID3D12Resource> cameraForGPUResource_ = nullptr;
	CameraForGPU *cameraForGPUData_ = nullptr;

	/*ComPtr<ID3D12Resource> essentialResources_ = nullptr;
	Essential* essentialData_ = nullptr;*/

	ComPtr<ID3D12Resource> objectMaterialResources_ = nullptr;
	ObjectMaterial *objectMaterialData_ = nullptr;

	Model *model_ = nullptr;
};