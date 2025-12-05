#include "Object3d.h"

#include "Object3dCommon.h"
#include "DxRootSignature.h"
#include "TextureManager.h"
#include "WIndow/WinApp.h"
#include "DxCommand.h"
#include "DxDevice.h"
#include "ModelManager.h"
#include "Camera/Camera.h"
#include "ImGuiManager.h"

Object3d::~Object3d()
{}

void Object3d::Initialize(Object3dCommon *obj3dCommon, const std::string &fileName)
{
	obj3dCommon_ = obj3dCommon;

	CreateResource();

	model_ = ModelManager::GetInstace().Load(fileName);
}

void Object3d::Initialize(Object3dCommon *obj3dCommon, Model *model)
{
	obj3dCommon_ = obj3dCommon;
	model_ = model;

	CreateResource();
}

void Object3d::Upadate()
{
	model_->SetTransform(transform_);
	if (camera_ == nullptr)
	{

		Vector3 scale = Vector3(1.0f, 1.0f, 1.0f);
		Vector3 translate = Vector3(0.0f, 0.0f, -1.0f);

		Matrix4x4 mainCameraMatrix = MakeAffineMatrix(scale, Vector3(), translate);
		Matrix4x4 mainCameraViewMatrix = MakeInVerse(mainCameraMatrix);
		Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(0.45f, static_cast<float>(WinApp::sWindoWidth) / static_cast<float>(WinApp::sWindoHeight), 0.1f, 100.0f);

		cameraForGPUData_->worldPosition = Vector3(0.0f, 0.0f, -10.0f);
	}
	else
	{
		cameraForGPUData_->worldPosition = camera_->GetPosition();

		model_->SetCamera(camera_);
	}

	model_->Update();
}

void Object3d::Draw()
{
	// 描画コマンドリストの取得
	ID3D12GraphicsCommandList *cmdList = obj3dCommon_->GetDirectXCommon()->GetCommand()->GetCommandList();
	DxRootSignature *dxRootSignature = obj3dCommon_->GetDxRootSignature();

	cmdList->SetGraphicsRootConstantBufferView(
		dxRootSignature->GetRootParamIndex(DxRootSignature::ParamSemanticType::CameraTransform),
		cameraForGPUResource_->GetGPUVirtualAddress()
	);
	cmdList->SetGraphicsRootConstantBufferView(
		dxRootSignature->GetRootParamIndex(DxRootSignature::ParamSemanticType::ObjectMaterial),
		objectMaterialResources_->GetGPUVirtualAddress()
	);

	DrawNode(model_->GetModelData().rootNode);


	//model_->Draw();
}

void Object3d::DebugWindow()
{
#ifdef USE_IMGUI
	if (model_->GetNumMeshies() <= 0) { return; }
	ImGui::Begin(model_->GetName().c_str());
	ImGui::DragFloat3("position", &transform_.translate.x, 0.01f);
	ImGui::DragFloat3("rotation", &transform_.rotate.x, 0.01f);
	ImGui::DragFloat3("scale", &transform_.scale.x, 0.01f);
	model_->SetTransform(transform_);

	ImGui::ColorEdit4("color", &objectMaterialData_->color.x);


	float shininess = model_->GetShininess();
	ImGui::DragFloat("shininess", &shininess, 0.1f, 1.0f, 100.0f);
	model_->SetShininess(shininess);

	ImGui::End();
#endif 
}

void Object3d::CreateResource()
{
	cameraForGPUResource_ = obj3dCommon_->GetDirectXCommon()->CreateBufferResource(sizeof(CameraForGPU));
	cameraForGPUResource_->Map(0, nullptr, reinterpret_cast<void **>(&cameraForGPUData_));

	objectMaterialResources_ = obj3dCommon_->GetDirectXCommon()->CreateBufferResource(sizeof(ObjectMaterial));
	objectMaterialResources_->Map(0, nullptr, reinterpret_cast<void **>(&objectMaterialData_));
	objectMaterialData_->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	objectMaterialData_->enableLighting = true;
}

void Object3d::DrawNode(const Model::Node &node)
{
	for (const auto &index : node.useMeshIndecies)
	{
		auto meshData = std::find_if(
			model_->GetModelData().meshes.begin(),
			model_->GetModelData().meshes.end(),
			[&](auto &m) { return m->meshPtr->GetOriginIndex() == index; }
		);

		auto *mesh = meshData->get();
		if (meshData != model_->GetModelData().meshes.end())
		{
			mesh->transformationMatrixData_->world = node.worldMatrix * model_->GetWorldMatrix();;
			mesh->transformationMatrixData_->worldInverseTranspose = MakeTransposeMatrix(MakeInVerse(mesh->transformationMatrixData_->world));
			if (camera_)
			{
				mesh->transformationMatrixData_->wvp = mesh->transformationMatrixData_->world * camera_->GetViewMatrix() * camera_->GetProjectionMatrix();
			}
			else
			{
				Matrix4x4 viewMatrix2D = MakeIdentityMatrix();
				Matrix4x4 projectionMatrix2D = MakeOrthoGraphicsMatrix(0.0f, 0.0f, static_cast<float>(WinApp::sWindoWidth), static_cast<float>(WinApp::sWindoHeight), 0.0f, 100.0f);
				mesh->transformationMatrixData_->wvp = mesh->transformationMatrixData_->world * viewMatrix2D * projectionMatrix2D;
			}

			ID3D12GraphicsCommandList *cmdList = obj3dCommon_->GetDirectXCommon()->GetCommand()->GetCommandList();
			const DxRootSignature *dxRootSignature = obj3dCommon_->GetDxRootSignature();
			cmdList->IASetVertexBuffers(0, 1, &mesh->meshPtr->GetGpuData()->vertexBufferViews_);
			cmdList->SetGraphicsRootConstantBufferView(
				dxRootSignature->GetRootParamIndex(DxRootSignature::ParamSemanticType::TransformationMatrix),
				mesh->transformationMatrixResource_->GetGPUVirtualAddress()
			);
			cmdList->SetGraphicsRootConstantBufferView(
				dxRootSignature->GetRootParamIndex(DxRootSignature::ParamSemanticType::MeshMaterial),
				mesh->meshPtr->GetGpuData()->materialResource->GetGPUVirtualAddress()
			);
			cmdList->SetGraphicsRootDescriptorTable(
				dxRootSignature->GetRootParamIndex(DxRootSignature::ParamSemanticType::Texture),
				mesh->meshPtr->GetGpuData()->texHandle_
			);
			cmdList->DrawInstanced(static_cast<int>(mesh->meshPtr->GetCpuData()->vertices.size()), 1, 0, 0);
			//mesh->meshPtr->Draw();
		}
	}
	// 更に階層を下る
	if (!node.children.empty())
	{
		for (const auto &child : node.children)
		{
			DrawNode(child);
		}
	}
}