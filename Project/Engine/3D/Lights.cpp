#include "Lights.h"
#include "DirectXCommon.h"
#include "DxDevice.h"
#include "DxCommand.h"

#include <iostream>
#include <numbers>
#include <algorithm>
#include <iterator>

void Lights::Initialize(DirectXCommon *dxCommon)
{
	if (dxCommon == nullptr) { return; }

	dxCommon_ = dxCommon;

	directionalLightResource_ = dxCommon->CreateBufferResource(sizeof(DirectionalLight));
	directionalLightResource_->Map(0, nullptr, reinterpret_cast<void **>(&directionalLightData_));
	directionalLightData_->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	directionalLightData_->direction = Vector3(0.0f, -1.0f, 0.0f);
	directionalLightData_->intensity = 1.0f;

	pointLightResource_ = dxCommon->CreateBufferResource(sizeof(PointLight)/* * sMaxPointLights*/);
	pointLightResource_->Map(0, nullptr, reinterpret_cast<void **>(&pointLightData_));
	pointLightData_->position = Vector3(0.0f, 2.0f, 0.0f);
	pointLightData_->color = Vector4(0.0f, 1.0f, 1.0f, 1.0f);
	pointLightData_->intensity = 20.0f;
	pointLightData_->radius = 3.0f;
	pointLightData_->decay = 1.0f;

	spotLightResource_ = dxCommon->CreateBufferResource(sizeof(SpotLight) /** sMaxSpotLights*/);
	spotLightResource_->Map(0, nullptr, reinterpret_cast<void **>(&spotLightData_));
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
	ID3D12GraphicsCommandList *commandList = dxCommon_->GetCommand()->GetCommandList();

	commandList->SetGraphicsRootConstantBufferView(3, directionalLightResource_->GetGPUVirtualAddress());
	commandList->SetGraphicsRootDescriptorTable(5, pointLightGPUHandle_);
	commandList->SetGraphicsRootDescriptorTable(6, spotLightGPUHandle_);
}

void Lights::CommandSet(const Vector3 &objectPos)
{
	std::vector<PointLight> filterdPointLight;
	std::copy_if(
		pointLights_.begin(),
		pointLights_.end(),
		std::back_inserter(filterdPointLight),
		[&](const PointLight light){
			return light.radius >= objectPos.Length();
		});
	PointLight::sNumLights = filterdPointLight.size();
	memcpy(pointLightData_, filterdPointLight.data(), filterdPointLight.size() * sizeof(PointLight));

	SpotLight::sNumLights;
}

void Lights::AllClear()
{
	pointLights_.clear();
	spotLights_.clear();
}

void Lights::CreateSRVHandle()
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDescPointLights{};
	srvDescPointLights.Format = DXGI_FORMAT_UNKNOWN;
	srvDescPointLights.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDescPointLights.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDescPointLights.Buffer.FirstElement = 0;
	srvDescPointLights.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	srvDescPointLights.Buffer.NumElements = Lights::sMaxPointLights;
	srvDescPointLights.Buffer.StructureByteStride = sizeof(Lights::PointLight);

	D3D12_CPU_DESCRIPTOR_HANDLE srvPointLightCpuHandle = dxCommon_->GetSRVDescriptorCPUHandle(2);
	pointLightGPUHandle_ = dxCommon_->GetSRVDescriptorGPUHandle(2);
	dxCommon_->GetDxDevice()->GetDevice()->CreateShaderResourceView(pointLightResource_.Get(), &srvDescPointLights, srvPointLightCpuHandle );

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDescSpotLights{};
	srvDescSpotLights.Format = DXGI_FORMAT_UNKNOWN;
	srvDescSpotLights.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDescSpotLights.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDescSpotLights.Buffer.FirstElement = 0;
	srvDescSpotLights.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	srvDescSpotLights.Buffer.NumElements = Lights::sMaxPointLights;
	srvDescSpotLights.Buffer.StructureByteStride = sizeof(Lights::PointLight);

	D3D12_CPU_DESCRIPTOR_HANDLE srvSpotLightCpuHandle = dxCommon_->GetSRVDescriptorCPUHandle(2);
	spotLightGPUHandle_= dxCommon_->GetSRVDescriptorGPUHandle(2);
	dxCommon_->GetDxDevice()->GetDevice()->CreateShaderResourceView(spotLightResource_.Get(), &srvDescSpotLights, srvSpotLightCpuHandle);
}