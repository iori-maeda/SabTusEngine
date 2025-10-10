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
		std::vector<std::pair<ObjectDataCPU, ObjectDataGPU>> objects;
		Node rootNode{};
	};

public:

	Model() = default;
	~Model();

	void Initialize(DirectXCommon* dxCommon, const std::string& directoryPath, const std::string& fileName);
	void Draw();

	static ModelData LoadFile(const std::string& directoryPath, const std::string &fileName);

public:

	std::string GetName() { return modelData_.modelName; }
	Vector4 GetColor() { return modelData_.objects[0].first.materialData->Kd; }
	float GetShininess() { return modelData_.objects[0].first.materialData->shininess; }

	void SetColor(const Vector4 &color)
	{
		for (auto &object : modelData_.objects)
		{
			ObjectDataCPU &objCPU = object.first;
			objCPU.materialData->Kd = color;
			objCPU.materialData->Ka = color / 0.5f;
		}
	}
	
	void SetEnableLighting(bool enableLighting)
	{
		for (auto &object : modelData_.objects)
		{
			ObjectDataCPU &objCPU = object.first;
			objCPU.materialData->enableLighting = enableLighting;
		}
	}

	void SetShininess(float shininess)
	{
		for (auto &object : modelData_.objects)
		{
			ObjectDataCPU &objCPU = object.first;
			objCPU.materialData->shininess = shininess;
		}
	}

private:

	void InitializeDataForGPU();

	static Model::MtlData ReadMaterial(aiMaterial* material);
	static Model::Node ReadNode(aiNode* node);

private:
	DirectXCommon *dxCommon_ = nullptr;

	ModelData modelData_{};
};