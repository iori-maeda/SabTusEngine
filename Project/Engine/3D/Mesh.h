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

	struct VertexData
	{
		Vector4 position{};
		Vector2 uv{};
		Vector3 normal{};
		//Vector3 tangent{};
		Vector4 tangent{};
	};

	struct TransformationMatrix
	{
		Matrix4x4 wvp{};
		Matrix4x4 world{};
		Matrix4x4 worldInverseTranspose{};
	};


	struct MaterialData
	{
		Vector4 Ka{};
		Vector4 Kd{};
		Vector4 Ks{};
		float shininess = 1000.0f;
		uint32_t isUseArmTex = false;
		uint32_t ambientOclusionChannel = 0;
		uint32_t roughnessChannel = 1;
		uint32_t metallicChannel = 2;
	};

	struct TextureInfo
	{
		std::string fileName;
		bool isSRGB = true;
	};

	struct MtlData
	{
		MaterialData material{};
		std::vector<TextureInfo> textureFilePaths;
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

	void Initialize(DirectXCommon *dxCommon);
	void Draw();

	bool ReadMesh(const aiScene *scene, uint32_t meshIndex);

public:

	std::string GetName()const { return name_; }
	Mesh::MeshDataCPU* GetCpuData() { return &meshCPU; }
	Mesh::MeshDataGPU* GetGpuData() { return &meshGPU; }
	int32_t GetOriginIndex()const { return originIndex_; }

	void SetParent(Mesh *parentPtr) { parent_ = parentPtr; }
	void SetAmbient(const Vector4 &ambient) { meshCPU.materialData->Ka = ambient; }
	void SetDiffuse(const Vector4 &diffuse) { meshCPU.materialData->Kd = diffuse; }
	void SetSpecular(const Vector4 &specular) { meshCPU.materialData->Ks = specular; }
	void SetShininess(float shininess) { meshCPU.materialData->shininess = shininess; }
	void SetTextureDirectoryPath(const std::string directoryPath) { textureDirectoryPath_ = directoryPath; }

private:

	bool ReadVertecies(aiMesh *mesh);
	bool ReadMtl(aiMaterial *material);
	bool InitializeGpuData();


private:

	DirectXCommon *dxCommon_ = nullptr;
	Mesh *parent_ = nullptr;

	std::string name_;
	MeshDataCPU meshCPU{};
	MeshDataGPU meshGPU{};

	std::string textureDirectoryPath_;

	int32_t originIndex_ = 0;
};