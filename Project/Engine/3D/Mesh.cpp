#include "Mesh.h"

#include <filesystem>
#include <cassert>
#include <format>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include "DirectXCommon.h"
#include "DxDevice.h"
#include "DxCommand.h"
#include "TextureManager.h"
#include "Logger.h"

Mesh::~Mesh()
{
	if (meshGPU.materialResource != nullptr)
	{
		meshGPU.materialResource->Unmap(0, nullptr);
	}
	if (meshGPU.vertexResource != nullptr)
	{
		meshGPU.vertexResource->Unmap(0, nullptr);
	}
}

void Mesh::Initialize(DirectXCommon *dxCommon)
{
	dxCommon_ = dxCommon;
	bool checkInitialize = InitializeGpuData();
	if (!checkInitialize)
	{
		Logger::Log("Mesh::loading failed");
	}
}

void Mesh::Draw()
{
	ID3D12GraphicsCommandList *cmdList = dxCommon_->GetCommand()->GetCommandList();

	cmdList->IASetVertexBuffers(0, 1, &meshGPU.vertexBufferViews_);
	cmdList->SetGraphicsRootConstantBufferView(0, meshGPU.materialResource->GetGPUVirtualAddress());
	cmdList->SetGraphicsRootDescriptorTable(2, meshGPU.texHandle_);
	cmdList->DrawInstanced(static_cast<int>(meshCPU.vertices.size()), 1, 0, 0);
}

bool Mesh::ReadMesh(const aiScene *scene, uint32_t meshIndex)
{
	if (scene == nullptr) { return false; }
	aiMesh *mesh = scene->mMeshes[meshIndex];
	if (mesh == nullptr) { return false; }
	name_ = mesh->mName.C_Str();
	ReadVertecies(mesh);

	aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
	if (material == nullptr) { return false; }
	ReadMtl(material);

	originIndex_ = static_cast<int32_t>(meshIndex);

	return true;
}

bool Mesh::ReadVertecies(aiMesh *mesh)
{
	if (mesh == nullptr) { return false; }
	// 法線とUV座標がないものは非対応
	//if (mesh->HasNormals() || mesh->HasTextureCoords(0)) { return false; }
	assert(mesh->HasNormals());
	assert(mesh->HasTextureCoords(0));

	for (uint32_t faceIndex = 0; faceIndex < mesh->mNumFaces; faceIndex++)
	{
		aiFace &face = mesh->mFaces[faceIndex];
		assert(face.mNumIndices == 3); // 三角形以外は非対応

		for (uint32_t element = 0; element < face.mNumIndices; element++)
		{
			uint32_t vertexIndex = face.mIndices[element];
			aiVector3D &position = mesh->mVertices[vertexIndex];
			aiVector3D &normal = mesh->mNormals[vertexIndex];
			aiVector3D &texcoord = mesh->mTextureCoords[0][vertexIndex];

			VertexData vertex{};
			vertex.position = Vector4(position.x, position.y, position.z, 1.0f);
			vertex.normal = Vector3(normal.x, normal.y, normal.z);
			vertex.uv = Vector2(texcoord.x, texcoord.y);

			meshCPU.vertices.push_back(vertex);
		}
	}

	return true;
}

bool Mesh::ReadMtl(aiMaterial *material)
{
	if (material == nullptr) { return false; }
	aiColor4D ambient{};
	if (material->Get(AI_MATKEY_COLOR_AMBIENT, ambient) == AI_SUCCESS)
	{
		meshCPU.mtlData.material.Ka = Vector4(ambient.r, ambient.g, ambient.b, ambient.a);
	}
	aiColor4D diffuse{};
	if (material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse) == AI_SUCCESS)
	{
		meshCPU.mtlData.material.Kd = Vector4(diffuse.r, diffuse.g, diffuse.b, diffuse.a);
	}
	aiColor4D specular{};
	if (material->Get(AI_MATKEY_COLOR_SPECULAR, specular) == AI_SUCCESS)
	{
		meshCPU.mtlData.material.Ks = Vector4(specular.r, specular.g, specular.b, specular.a);
	}
	float shininess = 0.0f;
	if (material->Get(AI_MATKEY_SHININESS, shininess) == AI_SUCCESS)
	{
		meshCPU.mtlData.material.shininess = shininess;
	}
	

	if (material->GetTextureCount(aiTextureType_DIFFUSE) != 0)
	{
		aiString texFilePath{};
		material->GetTexture(aiTextureType_DIFFUSE, 0, &texFilePath);
		meshCPU.mtlData.textureFilePath = texFilePath.C_Str();
		//meshCPU.mtlData.textureFilePath.push_back(texFilePath.C_Str());
	}
	return true;
}

bool Mesh::InitializeGpuData()
{
	if (dxCommon_ == nullptr) { return false; }

	meshGPU.vertexResource = dxCommon_->CreateBufferResource(sizeof(VertexData) * meshCPU.vertices.size());
	if (meshGPU.vertexResource == nullptr) { return false; }

	meshGPU.vertexResource->Map(0, nullptr, reinterpret_cast<void **>(&meshCPU.vertexData));
	memcpy(meshCPU.vertexData, meshCPU.vertices.data(), sizeof(VertexData) * meshCPU.vertices.size());

	meshGPU.vertexBufferViews_.BufferLocation = meshGPU.vertexResource->GetGPUVirtualAddress();
	meshGPU.vertexBufferViews_.SizeInBytes = static_cast<UINT>(meshCPU.vertices.size() * sizeof(VertexData));
	meshGPU.vertexBufferViews_.StrideInBytes = sizeof(VertexData);


	meshGPU.materialResource = dxCommon_->CreateBufferResource(sizeof(MaterialData));
	if (meshGPU.materialResource == nullptr) { return false; }

	meshGPU.materialResource->Map(0, nullptr, reinterpret_cast<void **>(&meshCPU.materialData));
	*meshCPU.materialData = meshCPU.mtlData.material;

	if (!textureDirectoryPath_.empty() && !meshCPU.mtlData.textureFilePath.empty())
	{
		TextureManager::GetInstace().Load(textureDirectoryPath_, meshCPU.mtlData.textureFilePath);
	}
	else
	{
		if (meshCPU.mtlData.textureFilePath.empty())
		{
			meshCPU.mtlData.textureFilePath = "whiteTest.png";
		}
		TextureManager::GetInstace().Load(meshCPU.mtlData.textureFilePath);
	}
	TextureDataCPU dataCpu = TextureManager::GetInstace().Load(meshCPU.mtlData.textureFilePath);
	meshGPU.texSize_ = Vector2(static_cast<float>(dataCpu.metaData.width), static_cast<float>(dataCpu.metaData.height));
	meshGPU.texHandle_ = TextureManager::GetInstace().GetSRVDescriptorGPUHandle(meshCPU.mtlData.textureFilePath);
	return true;
}