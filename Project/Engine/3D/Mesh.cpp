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
{}

void Mesh::Initialize(DirectXCommon *dxCommon, const std::string &name)
{
	dxCommon_ = dxCommon;
	name_ = name;
}

void Mesh::Draw()
{

}

bool Mesh::ReadMesh(aiScene* scene)
{
	if (scene == nullptr) { return false; }

	return false;
}

bool Mesh::ReadVertecies(aiMesh *mesh)
{
	if (mesh == nullptr) { return false; }
	// 法線とUV座標がないものは非対応
	if (mesh->HasNormals() || mesh->HasTextureCoords(0)) { return false; }
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

			objCPU.vertices.push_back(vertex);
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
		objCPU.mtlData.material.Ka = Vector4(ambient.r, ambient.g, ambient.b, ambient.a);
	}
	aiColor4D diffuse{};
	if (material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse) == AI_SUCCESS)
	{
		objCPU.mtlData.material.Kd = Vector4(diffuse.r, diffuse.g, diffuse.b, diffuse.a);
	}
	aiColor4D specular{};
	if (material->Get(AI_MATKEY_COLOR_SPECULAR, specular) == AI_SUCCESS)
	{
		objCPU.mtlData.material.Ks = Vector4(specular.r, specular.g, specular.b, specular.a);
	}

	if (material->GetTextureCount(aiTextureType_DIFFUSE) != 0)
	{
		aiString texFilePath{};
		material->GetTexture(aiTextureType_DIFFUSE, 0, &texFilePath);
		objCPU.mtlData.textureFilePath = texFilePath.C_Str();
	}
	return true;
}