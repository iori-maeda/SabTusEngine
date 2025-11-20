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

	resource_ = dxCommon->CreateBufferResource(sizeof(LightStatus) * Lights::sMaxLights);
	resource_->Map(0, nullptr, reinterpret_cast<void **>(&lightData_));

	LightStatus directionalLight{};
	directionalLight.type = DIRECTIONAL;
	directionalLight.color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	directionalLight.direction = Vector3(0.0f, -1.0f, 0.0f);
	directionalLight.intensity = 10.0f;
	lights_.push_back(directionalLight);

	LightStatus pointLight{};
	pointLight.type = POINT;
	pointLight.position = Vector3(0.0f, 2.0f, 0.0f);
	pointLight.color = Vector4(0.0f, 1.0f, 1.0f, 1.0f);
	pointLight.intensity = 20.0f;
	pointLight.range = 3.0f;
	pointLight.decay = 1.0f;
	lights_.push_back(pointLight);

	LightStatus spotLight{};
	spotLight.type = SPOT;
	spotLight.position = Vector3(-2.0f, 1.25f, 0.0f);
	spotLight.direction = Normalize(Vector3(1.0f, -1.0f, 0.0f));
	spotLight.color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	spotLight.intensity = 4.0f;
	spotLight.distance = 7.0f;
	spotLight.decay = 2.0f;
	spotLight.cosFallOffStart = std::cosf(std::numbers::pi_v<float> / 6.0f);
	spotLight.cosAngle = std::cosf(std::numbers::pi_v<float> / 3.0f);
	lights_.push_back(spotLight);

	CreateSRVHandle();
}

void Lights::SetDrawCommand()
{
	// 描画コマンドリストの取得
	ID3D12GraphicsCommandList *commandList = dxCommon_->GetCommand()->GetCommandList();

	commandList->SetGraphicsRootConstantBufferView(3, resource_->GetGPUVirtualAddress());
	commandList->SetGraphicsRootDescriptorTable(5, srvGPUHandle_);
}

uint32_t Lights::GetReflectLights(const Vector3 &objectPos)
{
	std::vector<LightStatus> filterdPointLight;
	std::copy_if(
		lights_.begin(),
		lights_.end(),
		std::back_inserter(filterdPointLight),
		[&](const LightStatus light)
		{
			return light.range >= objectPos.Length() || light.type == DIRECTIONAL;
		});

	memcpy(lightData_, filterdPointLight.data(), numLights_ * sizeof(LightStatus));
	return numLights_ = static_cast<uint32_t>(filterdPointLight.size());
}

void Lights::AllClear()
{
	lights_.clear();
}

void Lights::CreateSRVHandle()
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	srvDesc.Buffer.NumElements = Lights::sMaxLights;
	srvDesc.Buffer.StructureByteStride = sizeof(Lights::LightStatus);

	D3D12_CPU_DESCRIPTOR_HANDLE srvPointLightCpuHandle = dxCommon_->GetSRVDescriptorCPUHandle(2);
	srvGPUHandle_ = dxCommon_->GetSRVDescriptorGPUHandle(2);
	dxCommon_->GetDxDevice()->GetDevice()->CreateShaderResourceView(resource_.Get(), &srvDesc, srvPointLightCpuHandle);
}