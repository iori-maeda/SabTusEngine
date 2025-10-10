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

class DirectXCommon;

class Mesh
{
public:

	struct Node
	{
		Matrix4x4 localMatrix{};
		std::string name;
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

	struct MeshDataCPU
	{
		std::vector<VertexData> vertices;
		MtlData mtlData{};
		VertexData *vertexData = nullptr;
		MaterialData *materialData = nullptr;
	};

	struct MeshDataGPU
	{
		ComPtr<ID3D12Resource> vertexResource = nullptr;
		ComPtr<ID3D12Resource> materialResource = nullptr;
		D3D12_GPU_DESCRIPTOR_HANDLE texHandle_{};
		Vector2 texSize_{};
		D3D12_VERTEX_BUFFER_VIEW vertexBufferViews_{};
	};

public:

	Mesh() = default;
	~Mesh();

	void Initialize(DirectXCommon* dxCommon, const std::string &name);
	void Draw();

	bool ReadMesh(aiScene* scene);

public:

	std::string GetName()const { return name_; }
	Mesh::MeshDataCPU GetData()const { return objCPU; }

	void SetParent(Mesh *parentPtr) { parent_ = parentPtr; }

private:

	bool ReadVertecies(aiMesh *scene);
	bool ReadMtl(aiMaterial *material);


private:

	DirectXCommon *dxCommon_ = nullptr;

	Mesh *parent_ = nullptr;
	std::string name_;
	Node rootNode{};
	MeshDataCPU objCPU{};
	MeshDataGPU objGPU{};
};