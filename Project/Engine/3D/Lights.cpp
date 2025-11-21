#include "Lights.h"
#include "DirectXCommon.h"
#include "DxDevice.h"
#include "DxCommand.h"
#include "ImGuiManager/ImGuiManager.h"

#include <iostream>
#include <string>
#include <array>
#include <numbers>
#include <algorithm>
#include <iterator>

void Lights::Initialize(DirectXCommon *dxCommon)
{
	if (dxCommon == nullptr) { return; }

	dxCommon_ = dxCommon;
	lights_.reserve(sMaxLights);

	//AddLight(DIRECTIONAL);
	Lights::Light* newLight = AddLight(POINT);
	newLight->color = Vector4(1.0f, 0.0f, 0.0f, 1.0f);
	//AddLight(SPOT);
}

std::vector<Lights::Light> Lights::GetReflectLights(const Vector3 &objectPos)
{
	std::vector<std::pair<uint64_t, std::shared_ptr<Light>>> filterdPointLight;
	std::vector<Light> gpuSendLights;
	std::copy_if(
		lights_.begin(),
		lights_.end(),
		std::back_inserter(filterdPointLight),
		[&](auto &light)
		{
			return light.second->range >= objectPos.Length() || light.second->type == DIRECTIONAL;
		});
	std::transform(
		filterdPointLight.begin(),
		filterdPointLight.end(),
		std::back_inserter(gpuSendLights),
		[](auto &p)
		{
			return *p.second;
		});
	return gpuSendLights;
}

Lights::Light* Lights::AddLight(uint64_t lightType)
{
	std::shared_ptr<Lights::Light> newLight = std::make_shared<Lights::Light>();
	switch (lightType)
	{
	case DIRECTIONAL:
		newLight->type = DIRECTIONAL;
		newLight->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		newLight->direction = Vector3(0.0f, -1.0f, 0.0f);
		newLight->intensity = 10.0f;
		break;

	case POINT:
		newLight->type = POINT;
		newLight->position = Vector3(0.0f, 2.0f, 0.0f);
		newLight->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		newLight->intensity = 20.0f;
		newLight->range = 3.0f;
		newLight->decay = 1.0f;
		break;

	case SPOT:
		newLight->type = SPOT;
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
	lights_.push_back({ reinterpret_cast<std::uintptr_t>(newLight.get()), newLight });

	return lights_.back().second.get();
}

void Lights::DeleteLight(uint64_t lightId)
{
	auto deleteLight = std::remove_if(
		lights_.begin(),
		lights_.end(),
		[&](auto &light)
		{
			return light.first == lightId;
		});
	lights_.erase(deleteLight);
}

void Lights::AllClear()
{
	lights_.clear();
}

void Lights::DebugWindow()
{
#ifdef USE_IMGUI
	std::array<std::string, SUM_LIGHT_TYPE> lightName = { "Directional", "Point", "Spot" };
	ImGui::Begin("Lights");
	static uint64_t selectedType = DIRECTIONAL;

	if (ImGui::BeginCombo("Create Light Type", lightName[selectedType].c_str()))
	{
		for (uint64_t i = 0; i < SUM_LIGHT_TYPE; i++)
		{
			if (ImGui::Selectable(lightName[i].c_str(), true))
			{
				selectedType = i;
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}

	if (ImGui::Button("Add Light"))
	{
		AddLight(selectedType);
	}
	for (auto &light : lights_)
	{
		std::string widgetID = "##" + lightName[light.second->type] + "_" + std::to_string(light.first);
		std::string header = lightName[light.second->type] + "_" + std::to_string(light.first) + widgetID;

		std::string textColor = "color" + widgetID;
		std::string textDirection = "direction" + widgetID;
		std::string textIntensity = "intensity" + widgetID;
		std::string textPosition = "position" + widgetID;
		std::string textDistance = "distance" + widgetID;
		std::string textRange = "range" + widgetID;
		std::string textDecay = "decay" + widgetID;
		std::string textFalloff = "falloff" + widgetID;
		std::string textAngle = "angle" + widgetID;
		std::string textDelete = "delete" + widgetID;

		if (ImGui::CollapsingHeader(header.c_str()))
		{
			ImGui::Text("ptr=%p type=%d", &light, light.second->type);
			ImGui::ColorEdit4(textColor.c_str(), &light.second->color.x);
			ImGui::DragFloat(textIntensity.c_str(), &light.second->intensity, 0.01f, 0.0f);

			switch (light.second->type)
			{
			case DIRECTIONAL:
				ImGui::DragFloat3(textDirection.c_str(), &light.second->direction.x, 0.01f);
				light.second->direction = Normalize(light.second->direction);
				break;

			case POINT:
				ImGui::DragFloat3(textPosition.c_str(), &light.second->position.x, 0.01f);
				ImGui::DragFloat(textRange.c_str(), &light.second->range, 0.01f, 0.0f);
				ImGui::DragFloat(textDecay.c_str(), &light.second->decay, 0.01f, 0.0f);
				break;

			case SPOT:
				ImGui::DragFloat3(textPosition.c_str(), &light.second->position.x, 0.01f);
				ImGui::DragFloat(textRange.c_str(), &light.second->range, 0.01f, 0.0f);
				ImGui::DragFloat(textDecay.c_str(), &light.second->decay, 0.01f, 0.0f);
				ImGui::DragFloat3(textDirection.c_str(), &light.second->direction.x, 0.01f);
				ImGui::DragFloat(textDistance.c_str(), &light.second->distance, 0.01f, 0.0f);
				ImGui::DragFloat(textFalloff.c_str(), &light.second->cosFallOffStart, 0.001f);
				ImGui::DragFloat(textAngle.c_str(), &light.second->cosAngle, 0.001f);
				light.second->direction = Normalize(light.second->direction);
				/*light.second->cosFallOffStart = std::cosf(std::numbers::pi_v<float> / 6.0f);
				light.second->cosAngle = std::cosf(std::numbers::pi_v<float> / 3.0f);*/
				break;

			default:
				break;
			}

			if (ImGui::Button(textDelete.c_str()))
			{
				DeleteLight(light.first);
				break;
			}
		}
	}
	ImGui::End();
#endif
}