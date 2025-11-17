#include "Lights.h"
#include "DirectXCommon.h"
#include "DxCommand.h"

#include <numbers>

void Lights::Initialize(DirectXCommon* dxCommon)
{
	if (dxCommon == nullptr) { return; }

	dxCommon_ = dxCommon;

	directionalLightResource_ = dxCommon->CreateBufferResource(sizeof(DirectionalLight));
	directionalLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData_));
	directionalLightData_->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	directionalLightData_->direction = Vector3(0.0f, -1.0f, 0.0f);
	directionalLightData_->intensity = 1.0f;

	pointLightResource_ = dxCommon->CreateBufferResource(sizeof(PointLight)/* * sMaxPointLights*/);
	pointLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&pointLightData_));
	pointLightData_->position = Vector3(0.0f, 2.0f, 0.0f);
	pointLightData_->color = Vector4(0.0f, 1.0f, 1.0f, 1.0f);
	pointLightData_->intensity = 20.0f;
	pointLightData_->radius = 3.0f;
	pointLightData_->decay = 1.0f;

	spotLightResource_ = dxCommon->CreateBufferResource(sizeof(SpotLight) /** sMaxSpotLights*/);
	spotLightResource_->Map(0, nullptr, reinterpret_cast<void**>(&spotLightData_));
	spotLightData_->position = Vector3(-2.0f, 1.25f, 0.0f);
	spotLightData_->direction = Normalize(Vector3(1.0f, -1.0f, 0.0f));
	spotLightData_->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	spotLightData_->intensity = 4.0f;
	spotLightData_->distance = 7.0f;
	spotLightData_->decay = 2.0f;
	spotLightData_->cosFallOffStart = std::cosf(std::numbers::pi_v<float> / 6.0f);
	spotLightData_->cosAngle = std::cosf(std::numbers::pi_v<float> / 3.0f);
}

void Lights::CommandSet()
{
	// 描画コマンドリストの取得
	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommand()->GetCommandList();

	commandList->SetGraphicsRootConstantBufferView(3, directionalLightResource_->GetGPUVirtualAddress());
	commandList->SetGraphicsRootConstantBufferView(5, pointLightResource_->GetGPUVirtualAddress());
	commandList->SetGraphicsRootConstantBufferView(6, spotLightResource_->GetGPUVirtualAddress());
}

void Lights::AllClear()
{
	pointLights_.clear();
	spotLights_.clear();
}