#include "Object3d.h"

#include "Object3dCommon.h"
#include "TextureManager.h"
#include "WIndow/WinApp.h"
#include "DxCommand.h"
#include "ModelManager.h"
#include "Camera/Camera.h"
#include "ImGuiManager.h"

Object3d::~Object3d()
{
	transformationMatrixResource_->Unmap(0, nullptr);
}

void Object3d::Initiazlize(Object3dCommon* obj3dCommon, const std::string& fileName)
{
	obj3dCommon_ = obj3dCommon;

	transformationMatrixResource_ = obj3dCommon_->GetDirectXCommon()->CreateBufferResource(sizeof(TransformationMatrix));

	transformationMatrixResource_->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData_));
	Matrix4x4 viewMatrix2D = MakeIdentityMatrix();
	Matrix4x4 projectionMatrix2D = MakeOrthoGraphicsMatrix(0.0f, 0.0f, static_cast<float>(WinApp::kWindoWidth), static_cast<float>(WinApp::kWindoHeight), 0.0f, 100.0f);

	transformationMatrixData_->world = MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
	transformationMatrixData_->wvp = transformationMatrixData_->world * viewMatrix2D * projectionMatrix2D;
	transformationMatrixData_->worldInverseTranspose = MakeTransposeMatrix(MakeInVerse(transformationMatrixData_->world));

	cameraForGPUResource_ = obj3dCommon_->GetDirectXCommon()->CreateBufferResource(sizeof(CameraForGPU));
	cameraForGPUResource_->Map(0, nullptr, reinterpret_cast<void**>(&cameraForGPUData_));

	ModelManager::GetInstace().Initialize(obj3dCommon_->GetDirectXCommon());
	model_ = ModelManager::GetInstace().Load(fileName);
}

void Object3d::Upadate()
{

	transformationMatrixData_->world = MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
	transformationMatrixData_->worldInverseTranspose = MakeTransposeMatrix(MakeInVerse(transformationMatrixData_->world));
	if (camera_ == nullptr)
	{

		Vector3 scale = Vector3(1.0f, 1.0f, 1.0f);
		Vector3 translate = Vector3(0.0f, 0.0f, -1.0f);

		Matrix4x4 mainCameraMatrix = MakeAffineMatrix(scale, Vector3(), translate);
		Matrix4x4 mainCameraViewMatrix = MakeInVerse(mainCameraMatrix);
		Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(0.45f, static_cast<float>(WinApp::kWindoWidth) / static_cast<float>(WinApp::kWindoHeight), 0.1f, 100.0f);

		transformationMatrixData_->wvp = transformationMatrixData_->world * mainCameraViewMatrix * projectionMatrix;

		cameraForGPUData_->worldPosition = Vector3(0.0f, 0.0f, -10.0f);
	}
	else
	{
		transformationMatrixData_->wvp = transformationMatrixData_->world * camera_->GetViewMatrix() * camera_->GetProjectionMatrix();
		cameraForGPUData_->worldPosition = camera_->GetPosition();
	}
}

void Object3d::Draw()
{
	// 描画コマンドリストの取得
	ID3D12GraphicsCommandList* commandList = obj3dCommon_->GetDirectXCommon()->GetCommand()->GetCommandList();

	commandList->SetGraphicsRootConstantBufferView(1, transformationMatrixResource_->GetGPUVirtualAddress());
	commandList->SetGraphicsRootConstantBufferView(4, cameraForGPUResource_->GetGPUVirtualAddress());

	model_->Draw();
}

void Object3d::DebugWindow()
{
#ifdef USE_IMGUI
	ImGui::Begin(model_->GetName().c_str());
	ImGui::DragFloat3("position", &transform_.translate.x, 0.01f);
	ImGui::DragFloat3("rotation", &transform_.rotate.x, 0.01f);
	ImGui::DragFloat3("scale", &transform_.scale.x, 0.01f);

	Vector4 color = model_->GetColor();
	ImGui::ColorEdit4("color", &color.x);
	model_->SetColor(color);

	float shininess = model_->GetShininess();
	ImGui::DragFloat("shininess", &shininess, 0.1f, 1.0f, 100.0f);
	model_->SetShininess(shininess);

	ImGui::End();
#endif 
}