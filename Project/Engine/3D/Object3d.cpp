#include "Object3d.h"

#include "Object3dCommon.h"
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
	CreateLightsSRV();

	model_ = ModelManager::GetInstace().Load(fileName);
}

void Object3d::Initialize(Object3dCommon *obj3dCommon, Model *model)
{
	obj3dCommon_ = obj3dCommon;
	model_ = model;

	CreateResource();
	CreateLightsSRV();
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

	std::vector<Lights::Light> refrectLights = obj3dCommon_->GetLights()->GetReflectLights(transform_.translate);
	essentialData_->numLights = static_cast<uint32_t>(refrectLights.size());
	memcpy(lightData_, refrectLights.data(), refrectLights.size() * sizeof(Lights::Light));
}

void Object3d::Draw()
{
	// 描画コマンドリストの取得
	ID3D12GraphicsCommandList *commandList = obj3dCommon_->GetDirectXCommon()->GetCommand()->GetCommandList();
	commandList->SetGraphicsRootConstantBufferView(3, essentialResources_->GetGPUVirtualAddress());
	commandList->SetGraphicsRootConstantBufferView(4, cameraForGPUResource_->GetGPUVirtualAddress());
	commandList->SetGraphicsRootDescriptorTable(5, lightsSrvGPUHandle_);

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

void Object3d::CreateResource()
{
	cameraForGPUResource_ = obj3dCommon_->GetDirectXCommon()->CreateBufferResource(sizeof(CameraForGPU));
	cameraForGPUResource_->Map(0, nullptr, reinterpret_cast<void **>(&cameraForGPUData_));

	essentialResources_ = obj3dCommon_->GetDirectXCommon()->CreateBufferResource(sizeof(Essential));
	essentialResources_->Map(0, nullptr, reinterpret_cast<void **>(&essentialData_));

	lightsResource_ = obj3dCommon_->GetDirectXCommon()->CreateBufferResource(sizeof(Lights::Light) * Lights::sMaxLights);
	lightsResource_->Map(0, nullptr, reinterpret_cast<void **>(&lightData_));
}

void Object3d::CreateLightsSRV()
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	srvDesc.Buffer.NumElements = Lights::sMaxLights;
	srvDesc.Buffer.StructureByteStride = sizeof(Lights::Light);

	D3D12_CPU_DESCRIPTOR_HANDLE srvPointLightCpuHandle = obj3dCommon_->GetDirectXCommon()->GetSRVDescriptorCPUHandle(2);
	lightsSrvGPUHandle_ = obj3dCommon_->GetDirectXCommon()->GetSRVDescriptorGPUHandle(2);
	obj3dCommon_->GetDirectXCommon()->GetDxDevice()->GetDevice()->CreateShaderResourceView(lightsResource_.Get(), &srvDesc, srvPointLightCpuHandle);
}