#include "Lights.h"
#include "DirectXCommon.h"
#include "DxDevice.h"
#include "DxCommand.h"
#include "ImGuiManager/ImGuiManager.h"

#include <iostream>
#include <numbers>
#include <algorithm>
#include <iterator>

void Lights::Initialize(DirectXCommon* dxCommon)
{
	if (dxCommon == nullptr) { return; }

	dxCommon_ = dxCommon;

	lightName = { "Directional", "Point", "Spot" };

	// メモリ確保
	lights_.reserve(sMaxLights);

	// デフォルトライト追加
	Lights::Light* newLight = AddLight(Lights::LightType::DIRECTIONAL);
	newLight->direction = Normalize(Vector3(1.0f, 1.0, 1.0f));
	newLight->intensity = 3.0f;
}

std::vector<Lights::Light> Lights::GetReflectLights(const Vector3& objectPos)
{
	std::vector<EntryLight> filterdPointLight;
	std::vector<Light> gpuSendLights;
	std::copy_if(
		lights_.begin(),
		lights_.end(),
		std::back_inserter(filterdPointLight),
		[&](auto& light)
		{
			return light.data->range >= (objectPos - light.data->position).Length() || light.data->type == static_cast<uint32_t>(Lights::LightType::DIRECTIONAL);
		});
	std::transform(
		filterdPointLight.begin(),
		filterdPointLight.end(),
		std::back_inserter(gpuSendLights),
		[](auto& p)
		{
			return *p.data;
		});
	return gpuSendLights;
}

Lights::Light* Lights::AddLight(Lights::LightType type)
{
	std::shared_ptr<Lights::Light> newLight = std::make_shared<Lights::Light>();
	static uint64_t newLightID = 0;

	switch (type)
	{
	case Lights::LightType::DIRECTIONAL:
		newLight->type = static_cast<uint32_t>(Lights::LightType::DIRECTIONAL);
		newLight->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		newLight->direction = Vector3(0.0f, -1.0f, 0.0f);
		newLight->intensity = 10.0f;
		break;

	case Lights::LightType::POINT:
		newLight->type = static_cast<uint32_t>(Lights::LightType::POINT);
		newLight->position = Vector3(0.0f, 2.0f, 0.0f);
		newLight->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		newLight->intensity = 20.0f;
		newLight->range = 3.0f;
		newLight->decay = 1.0f;
		break;

	case Lights::LightType::SPOT:
		newLight->type = static_cast<uint32_t>(Lights::LightType::SPOT);
		newLight->position = Vector3(-2.0f, 1.25f, 0.0f);
		newLight->direction = Normalize(Vector3(1.0f, -1.0f, 0.0f));
		newLight->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		newLight->intensity = 4.0f;
		newLight->distance = 7.0f;
		newLight->decay = 2.0f;
		newLight->cosFallOffStart = std::cosf(std::numbers::pi_v<float> / 6.0f);
		newLight->cosAngle = std::cosf(std::numbers::pi_v<float> / 3.0f);
		break;

	default:

		break;
	}
	lights_.push_back({ newLightID++, newLight });

	return lights_.back().data.get();
}

void Lights::DeleteLight(Lights::LightType type)
{
	auto deleteLight = std::remove_if(
		lights_.begin(),
		lights_.end(),
		[&](auto& light)
		{
			return light.id == static_cast<uint32_t>(type);
		});
	lights_.erase(deleteLight, lights_.end());
}

void Lights::AllClear()
{
	lights_.clear();
}

void Lights::DebugWindow()
{
#ifdef USE_IMGUI
	ImGui::Begin("Lights");

	AddLightSelect();

	for (auto& light : lights_)
	{
		// 削除するID
		uint64_t* deleteID = nullptr;

		// ImGui用の内部ID
		std::string widgetID = "##" + lightName[light.data->type] + "_" + std::to_string(light.id);
		// ヘッダー名
		std::string header = lightName[light.data->type] + "_" + std::to_string(light.id);

		// ID設定 
		ImGui::PushID(widgetID.c_str());
		if (ImGui::CollapsingHeader(header.c_str()))
		{
			switch (static_cast<Lights::LightType>(light.data->type))
			{
			case Lights::LightType::DIRECTIONAL:
				CreeateDirectionalLightWindow(light.data.get());
				break;

			case Lights::LightType::POINT:
				CreeatePointLightWindow(light.data.get());
				break;

			case Lights::LightType::SPOT:
				CreeateSpotLightWindow(light.data.get());
				break;

			default:
				break;
			}

			if (ImGui::Button("delete"))
			{
				// 削除IDの登録
				deleteID = &light.id;
			}
		}
		ImGui::PopID();

		// 削除
		if (deleteID != nullptr) {
			DeleteLight(static_cast<Lights::LightType>(light.id));
			break;
		}
	}
	ImGui::End();
#endif
}

void Lights::AddLightSelect()
{
	static uint32_t selectedType = static_cast<uint32_t>(Lights::LightType::DIRECTIONAL);

	if (ImGui::BeginCombo("Create Light Type", lightName[selectedType].c_str()))
	{
		for (uint32_t i = 0; i < static_cast<uint32_t>(Lights::LightType::SUM_LIGHT_TYPE); i++)
		{
			bool isSelected = selectedType == i;
			if (ImGui::Selectable(lightName[i].c_str(), isSelected))
			{
				selectedType = i;
			}
			if (isSelected)
			{
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}

	if (ImGui::Button("Add Light"))
	{
		AddLight(static_cast<Lights::LightType>(selectedType));
	}
}

void Lights::CreeateDirectionalLightWindow(Light* light)
{
	if (light == nullptr) { return; }
	bool isChangedDirection = false;
	ImGui::Text("ptr=%p type=%d", light, light->type);
	ImGui::ColorEdit4("Color", &light->color.x);
	ImGui::DragFloat("Intensity", &light->intensity, 0.01f, 0.0f);
	isChangedDirection = ImGui::DragFloat3("Direction", &light->direction.x, 0.01f);
	if (isChangedDirection)
	{
		light->direction = Normalize(light->direction);
	}
}

void Lights::CreeatePointLightWindow(Light* light)
{
	if (light == nullptr) { return; }
	ImGui::Text("ptr=%p type=%d", light, light->type);
	ImGui::DragFloat3("Position", &light->position.x, 0.01f);
	ImGui::DragFloat("Intensity", &light->intensity, 0.01f, 0.0f);
	ImGui::ColorEdit4("Color", &light->color.x);
	ImGui::DragFloat("Range", &light->range, 0.01f, 0.0f);
	ImGui::DragFloat("Decay", &light->decay, 0.01f, 0.0f);
}

void Lights::CreeateSpotLightWindow(Light* light)
{
	if (light == nullptr) { return; }
	bool isChangedDirection = false;
	ImGui::Text("ptr=%p type=%d", light, light->type);
	ImGui::DragFloat3("Position", &light->position.x, 0.01f);
	isChangedDirection = ImGui::DragFloat3("Direction", &light->direction.x, 0.01f);
	ImGui::DragFloat("Intensity", &light->intensity, 0.01f, 0.0f);
	ImGui::ColorEdit4("Color", &light->color.x);
	ImGui::DragFloat("Range", &light->range, 0.01f, 0.0f);
	ImGui::DragFloat("Decay", &light->decay, 0.01f, 0.0f);
	ImGui::DragFloat("Distance", &light->distance, 0.01f, 0.0f);
	ImGui::DragFloat("Falloff", &light->cosFallOffStart, 0.001f);
	ImGui::DragFloat("Angle", &light->cosAngle, 0.001f);
	if (isChangedDirection)
	{
		light->direction = Normalize(light->direction);
	}
}
