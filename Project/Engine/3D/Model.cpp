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
#include "WIndow/WinApp.h"
#include "Camera/Camera.h"
#include "Logger.h"



Model::~Model()
{}

void Model::Initialize(DirectXCommon *dxCommon)
{
	if (dxCommon == nullptr) { return; }

	dxCommon_ = dxCommon;

	transform_.scale = Vector3(1.0f, 1.0f, 1.0f);
	transform_.rotate = {};
	transform_.translate = {};

	localMatrix_ = MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);

	for (auto &mesh : modelData_->meshes)
	{
		//mesh->meshPtr = std::make_unique<Mesh>();
		mesh->meshPtr->Initialize(dxCommon_);
		mesh->transformationMatrixResource_ = dxCommon_->CreateBufferResource(sizeof(TransformationMatrix));
		mesh->transformationMatrixResource_->Map(0, nullptr, reinterpret_cast<void **>(&mesh->transformationMatrixData_));

		mesh->transformationMatrixData_->world = MakeIdentityMatrix();
		mesh->transformationMatrixData_->wvp = MakeIdentityMatrix();
		mesh->transformationMatrixData_->worldInverseTranspose = MakeIdentityMatrix();
	}
}

void Model::Update()
{
	assert(dxCommon_ != nullptr);
	localMatrix_ = MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
}

void Model::Draw()
{
	assert(dxCommon_ != nullptr);

	for (auto &nodeChild : modelData_->rootNode.children)
	{
		for (auto &cNodeChild : nodeChild.children)
		{
			for (int32_t &index : cNodeChild.useMeshIndecies)
			{
				auto meshData = std::find_if(
					modelData_->meshes.begin(),
					modelData_->meshes.end(),
					[&](auto &m) { return m->meshPtr->GetOriginIndex() == index; }
				);

				auto* mesh = meshData->get();
				if (meshData != modelData_->meshes.end())
				{
					mesh->transformationMatrixData_->world = localMatrix_ * modelData_->rootNode.worldMatrix * cNodeChild.worldMatrix;
					mesh->transformationMatrixData_->worldInverseTranspose = MakeTransposeMatrix(MakeInVerse(mesh->transformationMatrixData_->world));
					if (camera_)
					{
						mesh->transformationMatrixData_->wvp = mesh->transformationMatrixData_->world * camera_->GetViewMatrix() * camera_->GetProjectionMatrix();
					}
					else
					{
						Matrix4x4 viewMatrix2D = MakeIdentityMatrix();
						Matrix4x4 projectionMatrix2D = MakeOrthoGraphicsMatrix(0.0f, 0.0f, static_cast<float>(WinApp::kWindoWidth), static_cast<float>(WinApp::kWindoHeight), 0.0f, 100.0f);
						mesh->transformationMatrixData_->wvp = mesh->transformationMatrixData_->world * viewMatrix2D * projectionMatrix2D;
					}

					ID3D12GraphicsCommandList *cmdList = dxCommon_->GetCommand()->GetCommandList();
					cmdList->SetGraphicsRootConstantBufferView(1, mesh->transformationMatrixResource_->GetGPUVirtualAddress());
					mesh->meshPtr->Draw();
				}
			}
		}
	}
}

bool Model::LoadModelFile(const std::string &directoryPath, const std::string &fileName)
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
		return false;
	}

	modelData_ = std::make_unique<ModelData>();

	for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; meshIndex++)
	{
		std::unique_ptr<MeshData> meshData = std::make_unique<MeshData>();
		meshData->meshPtr = std::make_unique<Mesh>();
		//meshData->meshPtr->Initialize(dxCommon_);
		meshData->meshPtr->SetTextureDirectoryPath(directoryPath);
		bool isMeshLoadCompleted = meshData->meshPtr->ReadMesh(scene, meshIndex);
		if (!isMeshLoadCompleted) { return false; }

		modelData_->meshes.push_back(std::move(meshData));
	}
	std::filesystem::path path(fileName);
	modelData_->modelName = path.stem().string();
	modelData_->rootNode = ReadNode(scene->mRootNode, MakeIdentityMatrix());

	return true;
}

Model::Node Model::ReadNode(aiNode *node, const Matrix4x4 &parentMatrix)
{
	Node result{};

	aiMatrix4x4 aiLocalMatrix = node->mTransformation;
	aiLocalMatrix = node->mTransformation;

	aiLocalMatrix.Transpose(); // 列ベクトルから行ベクトルに転置

	Matrix4x4 localMatrix{};
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			localMatrix.m[i][j] = aiLocalMatrix[i][j];
		}
	}

	result.worldMatrix = parentMatrix * localMatrix;

	result.name = node->mName.C_Str();
	result.children.resize(node->mNumChildren);

	for (uint32_t i = 0; i < node->mNumMeshes; i++)
	{
		result.useMeshIndecies.push_back(node->mMeshes[i]);
	}
	for (uint32_t childIndex = 0; childIndex < node->mNumChildren; childIndex++)
	{
		result.children[childIndex] = ReadNode(node->mChildren[childIndex], result.worldMatrix);
	}


	return result;
}