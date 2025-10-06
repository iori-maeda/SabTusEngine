#include "Model.h"

#include <fstream>
#include <sstream>
#include <filesystem>
#include <cassert>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "DirectXCommon.h"
#include "DxDevice.h"
#include "DxCommand.h"
#include "TextureManager.h"

std::string Model::defaultDirectoryPath = "Resources/Models/";

Model::~Model()
{
	for (auto &modelData : modelData_.objects)
	{
		modelData.second.materialResource->Unmap(0, nullptr);
		modelData.second.vertexResource->Unmap(0, nullptr);
	}
}

void Model::Initialize(DirectXCommon *dxCommon)
{
	dxCommon_ = dxCommon;

	modelData_ = LoadObjFile(defaultDirectoryPath, "axis.obj");
	for (auto &object : modelData_.objects)
	{
		ObjectDataCPU &objCPU = object.first;
		ObjectDataGPU &objGPU = object.second;
		if (objCPU.textureFilePath.empty())
		{
			objCPU.textureFilePath = "uvChecker.png";
		}

		objGPU.name = objCPU.name;

		TextureManager::GetInstace().Load(defaultDirectoryPath, objCPU.textureFilePath);
		objGPU.texHandle_ = TextureManager::GetInstace().GetSRVDescriptorGPUHandle(objCPU.textureFilePath);

		objGPU.vertexResource = dxCommon->CreateBufferResource(sizeof(VertexData) * objCPU.vertices.size());

		objGPU.vertexResource->Map(0, nullptr, reinterpret_cast<void **>(&objCPU.vertexData));
		memcpy(objCPU.vertexData, objCPU.vertices.data(), sizeof(VertexData) * objCPU.vertices.size());

		objGPU.vertexBufferViews_.BufferLocation = objGPU.vertexResource->GetGPUVirtualAddress();
		objGPU.vertexBufferViews_.SizeInBytes = static_cast<UINT>(objCPU.vertices.size() * sizeof(VertexData));
		objGPU.vertexBufferViews_.StrideInBytes = sizeof(VertexData);


		objGPU.materialResource = dxCommon->CreateBufferResource(sizeof(MaterialData));

		objGPU.materialResource->Map(0, nullptr, reinterpret_cast<void **>(&objCPU.materialData));
		*objCPU.materialData = modelData_.objects[0].first.mtlData;
		*objCPU.materialData = objCPU.mtlData;
		objCPU.materialData->enableLighting = true;
	}
}

void Model::Initialize(DirectXCommon *dxCommon, const std::string &fileName)
{
	dxCommon_ = dxCommon;

	modelData_ = LoadFile(defaultDirectoryPath, fileName);
	for (auto &object : modelData_.objects)
	{
		ObjectDataCPU &objCPU = object.first;
		ObjectDataGPU &objGPU = object.second;


		objGPU.name = objCPU.name;

		objGPU.texHandle_ = TextureManager::GetInstace().GetSRVDescriptorGPUHandle(objCPU.textureFilePath);

		objGPU.vertexResource = dxCommon->CreateBufferResource(sizeof(VertexData) * objCPU.vertices.size());

		objGPU.vertexResource->Map(0, nullptr, reinterpret_cast<void **>(&objCPU.vertexData));
		memcpy(objCPU.vertexData, objCPU.vertices.data(), sizeof(VertexData) * objCPU.vertices.size());

		objGPU.vertexBufferViews_.BufferLocation = objGPU.vertexResource->GetGPUVirtualAddress();
		objGPU.vertexBufferViews_.SizeInBytes = static_cast<UINT>(objCPU.vertices.size() * sizeof(VertexData));
		objGPU.vertexBufferViews_.StrideInBytes = sizeof(VertexData);


		objGPU.materialResource = dxCommon->CreateBufferResource(sizeof(MaterialData));

		objGPU.materialResource->Map(0, nullptr, reinterpret_cast<void **>(&objCPU.materialData));
		*objCPU.materialData = objCPU.mtlData;
		objCPU.materialData->enableLighting = true;
	}
}

void Model::Draw()
{
	assert(dxCommon_ != nullptr);
	// 描画コマンドリストの取得
	ID3D12GraphicsCommandList *commandList = dxCommon_->GetCommand()->GetCommandList();
	for (const auto &object : modelData_.objects)
	{
		const ObjectDataCPU &objCPU = object.first;
		const ObjectDataGPU &objGPU = object.second;

		commandList->IASetVertexBuffers(0, 1, &objGPU.vertexBufferViews_);
		commandList->SetGraphicsRootConstantBufferView(0, objGPU.materialResource->GetGPUVirtualAddress());
		commandList->SetGraphicsRootDescriptorTable(2, objGPU.texHandle_);
		commandList->DrawInstanced(static_cast<UINT>(objCPU.vertices.size()), 1, 0, 0);
	}
}

Model::ModelData Model::LoadObjFile(const std::string &directoryPath, const std::string &filePath)
{
	ModelData result;
	std::unique_ptr<ObjectDataCPU> obj = std::make_unique<ObjectDataCPU>();
	std::vector<Vector4> positions;
	std::vector<Vector3> normals;
	std::vector<Vector2> uvs;
	std::string useMtlFileName;

	std::string line;
	std::ifstream file(directoryPath + '/' + filePath);
	assert(file.is_open());

	while (std::getline(file, line))
	{
		std::string identifier;
		std::istringstream s(line);
		s >> identifier; // 先頭識別子読み込み

		if (identifier == "mtllib")
		{
			s >> useMtlFileName;
		}
		// オブジェクト名
		else if (identifier == "o")
		{
			// 名前が空じゃない = 内容がある
			// 返却用構造体にpuahして内容破棄
			if (!obj->name.empty())
			{
				result.objects.push_back(std::make_pair(*obj, ObjectDataGPU()));
				obj.release();
				obj = std::make_unique<ObjectDataCPU>();
			}
			s >> obj->name;
		}
		// 頂点
		else if (identifier == "v")
		{
			Vector4 position;
			s >> position.x >> position.y >> position.z;
			position.x *= -1.0f;
			position.w = 1.0f;
			positions.push_back(position);
		}
		// uv
		else if (identifier == "vt")
		{
			Vector2 uv;
			s >> uv.x >> uv.y;
			uv.y = 1.0f - uv.y;
			uvs.push_back(uv);
		}
		// 法線
		else if (identifier == "vn")
		{
			Vector3 normal;
			s >> normal.x >> normal.y >> normal.z;
			normal.x *= -1.0f;
			normals.push_back(normal);
		}
		// 使用マテリアル名
		else if (identifier == "usemtl")
		{
			std::string useMatreialName;
			s >> useMatreialName;
			MtlData useMtlData = LoadMtlFile(directoryPath + '/' + useMtlFileName, useMatreialName);
			obj->mtlData = useMtlData.material;
			obj->textureFilePath = useMtlData.textureFilePath;
		}
		// index
		else if (identifier == "f")
		{
			// とりあえず今は三角のみ
			const uint32_t triangleVertex = 3;
			VertexData triangle[triangleVertex];
			for (int32_t faceVertex = 0; faceVertex < triangleVertex; ++faceVertex)
			{
				std::string vertexDefinition;
				s >> vertexDefinition;
				// [頂点 / UV / 法線]で格納されているため分解してindexを取得
				std::istringstream v(vertexDefinition);
				uint32_t elementIndecies[3]{};
				for (int32_t element = 0; element < 3; ++element)
				{
					std::string index;
					std::getline(v, index, '/'); // スラッシュ区切りで読み込み
					elementIndecies[element] = std::stoi(index);
				}
				uint32_t positionIndex = elementIndecies[0] - 1;
				uint32_t uvIndex = elementIndecies[1] - 1;
				uint32_t normalIndex = elementIndecies[2] - 1;
				Vector4 position = positions[positionIndex];
				Vector2 uv = uvs[uvIndex];
				Vector3 normal = normals[normalIndex];
				triangle[faceVertex] = { position, uv, normal };
			}

			for (int i = triangleVertex - 1; i >= 0; --i)
			{
				obj->vertices.push_back(triangle[i]);
			}
		}
	}

	result.objects.push_back(std::make_pair(*obj, ObjectDataGPU()));
	// 最後にモデル名をfilePathから取得して終了
	std::filesystem::path path(filePath);
	result.modelName = path.stem().string();

	return result;
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
	assert(scene->HasMeshes());

	ModelData loadModel{};

	for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; meshIndex++)
	{
		ObjectDataCPU objCPU{};
		aiMesh *mesh = scene->mMeshes[meshIndex];

		// 法線とUV座標がないものは非対応
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
				/*vertex.position.x *= -1;
				vertex.normal.x *= -1;*/

				// meshの追加
			}
		}

		/*for (uint32_t materialIndex = 0; materialIndex < scene->mNumMaterials; materialIndex++)
		{*/
		aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
		if (material->GetTextureCount(aiTextureType_DIFFUSE) != 0)
		{
			aiString texFilePath{};
			material->GetTexture(aiTextureType_DIFFUSE, 0, &texFilePath);
			objCPU.textureFilePath = texFilePath.C_Str();
			TextureManager::GetInstace().Load(defaultDirectoryPath, objCPU.textureFilePath);
		}
		else
		{
			objCPU.textureFilePath = "whiteTest.png";
			TextureManager::GetInstace().Load(objCPU.textureFilePath);
		}

		aiColor4D ambient{};
		if (material->Get(AI_MATKEY_COLOR_AMBIENT, ambient) == AI_SUCCESS)
		{
			objCPU.mtlData.Ka = Vector4(ambient.r, ambient.g, ambient.b, ambient.a);
		}
		aiColor4D diffuse{};
		if (material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse) == AI_SUCCESS)
		{
			objCPU.mtlData.Kd = Vector4(diffuse.r, diffuse.g, diffuse.b, diffuse.a);
		}
		aiColor4D specular{};
		if (material->Get(AI_MATKEY_COLOR_SPECULAR, specular) == AI_SUCCESS)
		{
			objCPU.mtlData.Ks = Vector4(specular.r, specular.g, specular.b, specular.a);
		}
		//}
		loadModel.objects.push_back(std::make_pair(objCPU, ObjectDataGPU()));
	}
	std::filesystem::path path(fileName);
	loadModel.modelName = path.stem().string();

	return loadModel;
}

Model::MtlData Model::LoadMtlFile(const std::string &fileName, const std::string &useMaterialName)
{
	MtlData result{};

	std::string line;
	std::ifstream file(fileName);

	std::unique_ptr<std::string> materialName = std::make_unique<std::string>();

	while (std::getline(file, line))
	{
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;
		if (identifier == "newmtl")
		{
			s >> *materialName;
		}
		if (useMaterialName == *materialName)
		{
			if (identifier == "Ns")
			{

			}
			else if (identifier == "Ns")
			{

			}
			else if (identifier == "Ka")
			{
				s >> result.material.Ka.x >> result.material.Ka.y >> result.material.Ka.z;
				result.material.Ka.w = 1.0f;
			}
			else if (identifier == "Kd")
			{
				s >> result.material.Kd.x >> result.material.Kd.y >> result.material.Kd.z;
				result.material.Kd.w = 1.0f;
			}
			else if (identifier == "Ks")
			{
				s >> result.material.Ks.x >> result.material.Ks.y >> result.material.Ks.z;
				result.material.Ks.w = 1.0f;
			}
			else if (identifier == "Ke")
			{

			}
			else if (identifier == "Ni")
			{

			}
			else if (identifier == "ilumi")
			{

			}
			else if (identifier == "map_Kd")
			{
				s >> result.textureFilePath;
			}
		}
	}
	return result;
}