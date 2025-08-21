#include "Model.h"

#include <fstream>
#include <sstream>
#include <cassert>

#include "../DirectXCommon.h"
#include "../DirectX12Objects/DxDevice.h"
#include "../DirectX12Objects/DxCommand.h"
#include "../TextureManager.h"

void Model::Initialize(DirectXCommon* dxCommon)
{
	dxCommon_ = dxCommon;

	modelData_ = LoadObjFile("Resources/Models", "axis.obj");
	for (auto& object : modelData_.objects)
	{
		ObjectDataCPU& objCPU = object.first;
		ObjectDataGPU& objGPU = object.second;
		if (objCPU.textureFilePath.empty())
		{
			objCPU.textureFilePath = "uvChecker.png";
		}

		objGPU.name = objCPU.name;

		TextureManager::GetInstace().Load("Resources/Models/", objCPU.textureFilePath);
		objGPU.texHandle_ = TextureManager::GetInstace().GetSRVDescriptorGPUHandle(objCPU.textureFilePath);

		objGPU.vertexResource = dxCommon->CreateBufferResource(sizeof(VertexData) * objCPU.vertices.size());

		objGPU.vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&objCPU.vertexData));
		memcpy(objCPU.vertexData, objCPU.vertices.data(), sizeof(VertexData) * objCPU.vertices.size());

		objGPU.vertexBufferViews_.BufferLocation = objGPU.vertexResource->GetGPUVirtualAddress();
		objGPU.vertexBufferViews_.SizeInBytes = static_cast<UINT>(objCPU.vertices.size() * sizeof(VertexData));
		objGPU.vertexBufferViews_.StrideInBytes = sizeof(VertexData);


		objGPU.materialResource = dxCommon->CreateBufferResource(sizeof(MaterialData));

		objGPU.materialResource->Map(0, nullptr, reinterpret_cast<void**>(&objCPU.materialData));
		*objCPU.materialData = modelData_.objects[0].first.material;
		objCPU.materialData->Kd = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		objCPU.materialData->enableLighting = true;
	}
}

void Model::Initialize(DirectXCommon* dxCommon, const std::string& fileName)
{
	dxCommon_ = dxCommon;

	modelData_ = LoadObjFile("Resources/Models", fileName);
	for (auto& object : modelData_.objects)
	{
		ObjectDataCPU& objCPU = object.first;
		ObjectDataGPU& objGPU = object.second;
		if (objCPU.textureFilePath.empty())
		{
			objCPU.textureFilePath = "uvChecker.png";
		}

		objGPU.name = objCPU.name;

		TextureManager::GetInstace().Load("Resources/Models/", objCPU.textureFilePath);
		objGPU.texHandle_ = TextureManager::GetInstace().GetSRVDescriptorGPUHandle(objCPU.textureFilePath);

		objGPU.vertexResource = dxCommon->CreateBufferResource(sizeof(VertexData) * objCPU.vertices.size());

		objGPU.vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&objCPU.vertexData));
		memcpy(objCPU.vertexData, objCPU.vertices.data(), sizeof(VertexData) * objCPU.vertices.size());

		objGPU.vertexBufferViews_.BufferLocation = objGPU.vertexResource->GetGPUVirtualAddress();
		objGPU.vertexBufferViews_.SizeInBytes = static_cast<UINT>(objCPU.vertices.size() * sizeof(VertexData));
		objGPU.vertexBufferViews_.StrideInBytes = sizeof(VertexData);


		objGPU.materialResource = dxCommon->CreateBufferResource(sizeof(MaterialData));

		objGPU.materialResource->Map(0, nullptr, reinterpret_cast<void**>(&objCPU.materialData));
		*objCPU.materialData = modelData_.objects[0].first.material;
		objCPU.materialData->Kd = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		objCPU.materialData->enableLighting = true;
	}
}

void Model::Draw()
{
	assert(dxCommon_ != nullptr);
	// 描画コマンドリストの取得
	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommand()->GetCommandList();
	for (const auto& object : modelData_.objects)
	{
		const ObjectDataCPU& objCPU = object.first;
		const ObjectDataGPU& objGPU = object.second;
		commandList->IASetVertexBuffers(0, 1, &objGPU.vertexBufferViews_);
		commandList->SetGraphicsRootConstantBufferView(0, objGPU.materialResource->GetGPUVirtualAddress());
		commandList->SetGraphicsRootDescriptorTable(2, objGPU.texHandle_);
		commandList->DrawInstanced(static_cast<UINT>(objCPU.vertices.size()), 1, 0, 0);
	}
}

Model::ModelData Model::LoadObjFile(const std::string& directoryPath, const std::string& filePath)
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
		else if (identifier == "o") {
			// 名前が空じゃない = 内容がある
			// 返却用構造体にpuahして内容破棄
			if (!obj->name.empty()) {
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
		else if (identifier == "usemtl") {
			std::string useMatreialName;
			s >> useMatreialName;
			MtlData useMtlData = LoadMtlFile(directoryPath + '/' + useMtlFileName, useMatreialName);
			obj->material = useMtlData.material;
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
	return result;
}

Model::MtlData Model::LoadMtlFile(const std::string& fileName, const std::string& useMaterialName)
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