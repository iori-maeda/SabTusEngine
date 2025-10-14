#include "Model.h"

#include <fstream>
#include <sstream>
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



Model::~Model()
{
	/*for (auto &modelData : modelData_.objects)
	{
		modelData.second.materialResource->Unmap(0, nullptr);
		modelData.second.vertexResource->Unmap(0, nullptr);
	}*/
}

void Model::Initialize(DirectXCommon *dxCommon, const std::string &directoryPath, const std::string &fileName)
{
	dxCommon_ = dxCommon;

	modelData_ = LoadFile(directoryPath, fileName);
	//InitializeDataForGPU();
	for(auto& mesh : modelData_.meshies_)
	{
		mesh->Initialize(dxCommon);
	}
}

void Model::Draw()
{
	assert(dxCommon_ != nullptr);
	// 描画コマンドリストの取得
	//ID3D12GraphicsCommandList *commandList = dxCommon_->GetCommand()->GetCommandList();
	for (const auto &mesh : modelData_.meshies_)
	{
		mesh->Draw();
	}
}

Model::ModelData Model::LoadFile(const std::string &directoryPath, const std::string &fileName)
{
	Assimp::Importer importer;
	uint32_t readFlag =
		aiProcess_FlipWindingOrder |
		aiProcess_FlipUVs |
		aiProcess_Triangulate |
		aiProcess_MakeLeftHanded |
		aiProcess_CalcTangentSpace;
	const aiScene *scene = importer.ReadFile((directoryPath + fileName).c_str(), readFlag); // 三角形逆順、UVフリップオプションで読み込み
	if (!scene)
	{
		std::string error = importer.GetErrorString();
		Logger::Log(std::format("Assimp Error: {}\n", error));
	}
	assert(scene->HasMeshes());

	//Mesh loadModel{};
	ModelData loadModel{};

	for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; meshIndex++)
	{
		std::unique_ptr<Mesh> meshData = std::make_unique<Mesh>();
		//aiMesh *mesh = scene->mMeshes[meshIndex];
		meshData->ReadMesh(scene, meshIndex);
		//meshData->Initialize(dxCommon_, )

		/*meshData.name = mesh->mName.C_Str();
		meshData.vertices = ReadVerticies(mesh);*/

		//aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
		/*meshData.mtlData = ReadMaterial(material);*/

		/*if (!meshData.mtlData.textureFilePath.empty())
		{
			TextureManager::GetInstace().Load(directoryPath, meshData.mtlData.textureFilePath);
		}
		else
		{
			meshData.mtlData.textureFilePath = "whiteTest.png";
			TextureManager::GetInstace().Load(meshData.mtlData.textureFilePath);
		}
		*/
		loadModel.rootNode = ReadNode(scene->mRootNode);
		loadModel.meshies_.push_back(std::move(meshData));
	}
	std::filesystem::path path(fileName);
	loadModel.modelName = path.stem().string();

	return loadModel;
}

//void Model::InitializeDataForGPU()
//{
//	for (auto &object : modelData_.objects)
//	{
//		ObjectDataCPU &objCPU = object.first;
//		ObjectDataGPU &objGPU = object.second;
//
//
//		objGPU.name = objCPU.name;
//
//		objGPU.texHandle_ = TextureManager::GetInstace().GetSRVDescriptorGPUHandle(objCPU.mtlData.textureFilePath);
//
//		objGPU.vertexResource = dxCommon_->CreateBufferResource(sizeof(VertexData) * objCPU.vertices.size());
//
//		objGPU.vertexResource->Map(0, nullptr, reinterpret_cast<void **>(&objCPU.vertexData));
//		memcpy(objCPU.vertexData, objCPU.vertices.data(), sizeof(VertexData) * objCPU.vertices.size());
//
//		objGPU.vertexBufferViews_.BufferLocation = objGPU.vertexResource->GetGPUVirtualAddress();
//		objGPU.vertexBufferViews_.SizeInBytes = static_cast<UINT>(objCPU.vertices.size() * sizeof(VertexData));
//		objGPU.vertexBufferViews_.StrideInBytes = sizeof(VertexData);
//
//
//		objGPU.materialResource = dxCommon_->CreateBufferResource(sizeof(MaterialData));
//
//		objGPU.materialResource->Map(0, nullptr, reinterpret_cast<void **>(&objCPU.materialData));
//		*objCPU.materialData = objCPU.mtlData.material;
//		objCPU.materialData->enableLighting = true;
//	}
//}

std::vector<Model::VertexData> Model::ReadVerticies(aiMesh *mesh)
{
	std::vector<VertexData>verticies;

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

			verticies.push_back(vertex);

			// meshの追加
		}
	}
	return verticies;
}

Model::MtlData Model::ReadMaterial(aiMaterial *material)
{
	MtlData result{};

	aiColor4D ambient{};
	if (material->Get(AI_MATKEY_COLOR_AMBIENT, ambient) == AI_SUCCESS)
	{
		result.material.Ka = Vector4(ambient.r, ambient.g, ambient.b, ambient.a);
	}
	aiColor4D diffuse{};
	if (material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse) == AI_SUCCESS)
	{
		result.material.Kd = Vector4(diffuse.r, diffuse.g, diffuse.b, diffuse.a);
	}
	aiColor4D specular{};
	if (material->Get(AI_MATKEY_COLOR_SPECULAR, specular) == AI_SUCCESS)
	{
		result.material.Ks = Vector4(specular.r, specular.g, specular.b, specular.a);
	}

	if (material->GetTextureCount(aiTextureType_DIFFUSE) != 0)
	{
		aiString texFilePath{};
		material->GetTexture(aiTextureType_DIFFUSE, 0, &texFilePath);
		result.textureFilePath = texFilePath.C_Str();
	}
	return result;
}

Model::Node Model::ReadNode(aiNode *node)
{
	Node result{};

	aiMatrix4x4 aiLocalMatrix = node->mTransformation;
	aiLocalMatrix.Transpose(); // 列ベクトルから行ベクトルに転置

	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			result.localMatrix.m[i][j] = aiLocalMatrix[i][j];
		}
	}
	result.name = node->mName.C_Str();
	result.children.resize(node->mNumChildren);
	for (uint32_t childIndex = 0; childIndex < node->mNumChildren; childIndex++)
	{
		result.children[childIndex] = ReadNode(node->mChildren[childIndex]);
	}


	return result;
}
