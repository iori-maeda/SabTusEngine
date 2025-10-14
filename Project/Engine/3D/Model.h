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

class Model
{
public:
	struct Node
	{
		Matrix4x4 localMatrix{};
		std::string name;;
		std::vector<Node>children;
	};

	struct VertexData
	{
		Vector4 position{};
		Vector2 uv{};
		Vector3 normal{};
	};

	struct MaterialData
	{
		Vector4 Ka{};
		Vector4 Kd{};
		Vector4 Ks{};
		float shininess = 1.0f;
		int32_t enableLighting = true;
	};

	struct MtlData
	{
		MaterialData material{};
		std::string textureFilePath;
	};

	struct ObjectDataCPU
	{
		std::string name;
		std::vector<VertexData> vertices;
		MtlData mtlData{};
		VertexData *vertexData = nullptr;
		MaterialData *materialData = nullptr;
	};

	struct ObjectDataGPU
	{
		std::string name;
		ComPtr<ID3D12Resource> vertexResource = nullptr;
		ComPtr<ID3D12Resource> materialResource = nullptr;
		D3D12_GPU_DESCRIPTOR_HANDLE texHandle_{};
		Vector2 texSize_{};
		D3D12_VERTEX_BUFFER_VIEW vertexBufferViews_{};
	};



	struct ModelData
	{
		std::string modelName;
		std::vector <std::unique_ptr<Mesh>> meshies_;
		Node rootNode{};
	};

public:

	Model() = default;
	~Model();

	void Initialize(DirectXCommon *dxCommon, const std::string &directoryPath, const std::string &fileName);
	void Draw();

	static ModelData LoadFile(const std::string &directoryPath, const std::string &fileName);

public:

	std::string GetName() { return modelData_.modelName; }
	size_t GetNumMeshies() { return modelData_.meshies_.size(); }
	Vector4 GetColor() { return modelData_.meshies_[0]->GetData().materialData->Kd; }
	float GetShininess() { return modelData_.meshies_[0]->GetData().materialData->shininess; }

	void SetColor(const Vector4 &color)
	{
		for (auto &mesh : modelData_.meshies_)
		{
			mesh->SetDiffuse(color);
			mesh->SetAmbient(color / 2);
		}
	}

	void SetEnableLighting(bool enableLighting)
	{
		for (auto &mesh : modelData_.meshies_)
		{
			mesh->SetEnableLighting(enableLighting);
		}
	}

	void SetShininess(float shininess)
	{
		for (auto &mesh : modelData_.meshies_)
		{
			mesh->SetShininess(shininess);
		}
	}

private:

	//void InitializeDataForGPU();

	static std::vector<Model::VertexData> ReadVerticies(aiMesh *mesh);
	static Model::MtlData ReadMaterial(aiMaterial *material);
	static Model::Node ReadNode(aiNode *node);

private:
	DirectXCommon *dxCommon_ = nullptr;

	ModelData modelData_{};
};