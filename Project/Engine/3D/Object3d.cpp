#include "Object3d.h"

#include "Object3dCommon.h"
#include "TextureManager.h"
#include "WIndow/WinApp.h"
#include "DxCommand.h"
#include "ModelManager.h"
#include "Camera/Camera.h"
#include "Lights.h"
#include "ImGuiManager.h"

Object3d::~Object3d()
{}

void Object3d::Initialize(Object3dCommon *obj3dCommon, const std::string &fileName)
{
	obj3dCommon_ = obj3dCommon;

	cameraForGPUResource_ = obj3dCommon_->GetDirectXCommon()->CreateBufferResource(sizeof(CameraForGPU));
	cameraForGPUResource_->Map(0, nullptr, reinterpret_cast<void **>(&cameraForGPUData_));

	essentialResources_ = obj3dCommon_->GetDirectXCommon()->CreateBufferResource(sizeof(Essential));
	essentialResources_->Map(0, nullptr, reinterpret_cast<void **>(&essentialData_));

	model_ = ModelManager::GetInstace().Load(fileName);
}

void Object3d::Initialize(Object3dCommon *obj3dCommon, Model *model)
{
	obj3dCommon_ = obj3dCommon;
	model_ = model;

	cameraForGPUResource_ = obj3dCommon_->GetDirectXCommon()->CreateBufferResource(sizeof(CameraForGPU));
	cameraForGPUResource_->Map(0, nullptr, reinterpret_cast<void **>(&cameraForGPUData_));

	essentialResources_ = obj3dCommon_->GetDirectXCommon()->CreateBufferResource(sizeof(Essential));
	essentialResources_->Map(0, nullptr, reinterpret_cast<void **>(&essentialData_));
}

void Object3d::Upadate()
{
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
	ID3D12GraphicsCommandList *commandList = obj3dCommon_->GetDirectXCommon()->GetCommand()->GetCommandList();
	essentialData_->numLights = obj3dCommon_->GetLights()->GetReflectLights(transform_.translate);
	obj3dCommon_->GetLights()->SetDrawCommand();
	commandList->SetGraphicsRootConstantBufferView(3, essentialResources_->GetGPUVirtualAddress());
	commandList->SetGraphicsRootConstantBufferView(4, cameraForGPUResource_->GetGPUVirtualAddress());

	model_->Draw();
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

	Vector4 color = model_->GetColor();
	ImGui::ColorEdit4("color", &color.x);
	model_->SetColor(color);

	float shininess = model_->GetShininess();
	ImGui::DragFloat("shininess", &shininess, 0.1f, 1.0f, 100.0f);
	model_->SetShininess(shininess);

	ImGui::End();
#endif 
}