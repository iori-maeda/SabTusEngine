#pragma once
#include <d3d12.h>
#include <cstdint>
#include <string>
#include <vector>
#include <memory>

#include <assimp/scene.h>

#include "ComPtr.h"
#include "Math/Vector4.h"
#include "Math/Vector3.h"
#include "Math/Vector2.h"
#include "Math/Matrix4x4.h"

#include "Mesh.h"

class DirectXCommon;
class Camera;

class Model
{
public:

	struct Node
	{
		Matrix4x4 worldMatrix{};
		std::string name;
		std::vector<Node>children;
		std::vector<int32_t> useMeshIndecies;
	};

	struct Transform
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

	struct MeshData
	{
		std::unique_ptr<Mesh> meshPtr = nullptr;
		ComPtr<ID3D12Resource> transformationMatrixResource_ = nullptr;
		TransformationMatrix* transformationMatrixData_ = nullptr;
	};

	struct ModelData
	{
		std::string modelName;
		std::vector <std::unique_ptr<MeshData>> meshes;
		Node rootNode{};
	};

public:

	Model() = default;
	~Model();

	void Initialize(DirectXCommon* dxCommon);
	void Update();
	void Draw();

	bool LoadModelFile(const std::string& directoryPath, const std::string& fileName);

public:

	std::string GetName() { return modelData_->modelName; }
	size_t GetNumMeshies() { return modelData_->meshes.size(); }
	Mesh* GetMesh(size_t index) { return index < modelData_->meshes.size() ? modelData_->meshes[index]->meshPtr.get() : nullptr; }
	//Vector4 GetColor() { return modelData_->meshes[0]->meshPtr->GetData().materialData->Kd; }
	float GetShininess() { return modelData_->meshes[0]->meshPtr->GetCpuData().materialData->shininess; }

	void SetCamera(Camera* camera) { camera_ = camera; }
	void SetTransform(const Model::Transform& transform) { transform_ = transform; }
	void SetColor(const Vector4& color)
	{
		for (auto& mesh : modelData_->meshes)
		{
			mesh->meshPtr->SetDiffuse(color);
			mesh->meshPtr->SetAmbient(color / 2);
		}
	}

	void SetShininess(float shininess)
	{
		for (auto& mesh : modelData_->meshes)
		{
			mesh->meshPtr->SetShininess(shininess);
		}
	}

private:
	Model::Node ReadNode(aiNode* node, const Matrix4x4& parentMatrix);
	void DrawNode(const Node& node);

private:
	DirectXCommon* dxCommon_ = nullptr;
	Camera* camera_ = nullptr;

	std::unique_ptr<ModelData> modelData_ = nullptr;

	Transform transform_{};
	// モデルの変換行列
	Matrix4x4 modelMatrix_{};
};