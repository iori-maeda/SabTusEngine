#include "SabTusFramework.h"
#include "ImGuiManager.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include "ParticleSystem/ParticleSystem.h"
#include "WinApp.h"
#include "DirectXCommon.h"
#include "2D/SpriteCommon.h"
#include "3D/Object3dCommon.h"
#include "Camera/Camera.h"
#include "IO/Input.h"
#include "FrameRateController.h"
#include "3D/Lights.h"

SabTusFramework::~SabTusFramework()
{}

void SabTusFramework::Initialize()
{
#pragma region SystemVaiable
	winApp_ = std::make_unique<WinApp>();
	WinApp::sWindoWidth = 1280;
	WinApp::sWindoHeight = 720;
	winApp_->Initialize();

	dxCommon_ = std::make_unique<DirectXCommon>();
	dxCommon_->Initialize(*winApp_.get());

	ImGuiManager::Initialize(winApp_.get(), dxCommon_.get());

	TextureManager::GetInstace().Initialize(dxCommon_.get());

	ModelManager::GetInstace().Initialize(dxCommon_.get());

	spriteCommon_ = std::make_unique<SpriteCommon>();
	spriteCommon_->Initialize(dxCommon_.get());

	object3dCommon_ = std::make_unique<Object3dCommon>();
	object3dCommon_->Initialize(dxCommon_.get());

	input_ = std::make_unique<Input>();
	input_->Initialize(winApp_.get());

	mainCamera_ = std::make_unique<Camera>();
	mainCamera_->Initialize();
	mainCamera_->SetPosition(Vector3(0.0f, 5.0f, -15.0f));
	mainCamera_->SetRotation(Vector3(0.26f, 0.0f, 0.0f));
	mainCamera_->SetInput(input_.get());

	ParticleSystem::GetInstance()->Initialize(dxCommon_.get());
	ParticleSystem::GetInstance()->SetCamera(mainCamera_.get());

	fpsController_ = std::make_unique<FrameRateController>();
	fpsController_->Initialize();

	lights_ = std::make_unique<Lights>();
	lights_->Initialize(dxCommon_.get(), mainCamera_.get());
	object3dCommon_->SetLights(lights_.get());
#pragma endregion
}

void SabTusFramework::Update()
{
	winApp_->Update();
	fpsController_->Update();
	input_->UpdateAllDevice();

	DebugWindow();

	mainCamera_->DebugCameraMode();
	mainCamera_->Update();

	lights_->Update();
}

void SabTusFramework::Finalize()
{
	ImGuiManager::Finalize();
	ModelManager::GetInstace().Finalize();
	TextureManager::GetInstace().Finalize();

	ParticleSystem::GetInstance()->Finalize();

	winApp_->Finalize();
}

bool SabTusFramework::EndRequest()
{
	return winApp_->ProccesMessage() || input_->TriggerKey(DIK_ESCAPE);
}

void SabTusFramework::DebugWindow()
{
#ifdef _DEBUG
#ifdef USE_IMGUI
	ImGuiManager::Begin();
	fpsController_->DebugWindow();
	mainCamera_->DebugWindow();
	object3dCommon_->DebugWindow();
	/*object3d_->DebugWindow();
	object3d2_->DebugWindow();
	ImGui::Begin("GameDebugWinDow");
	ImGui::DragFloat3("Emit Position", &emitPosition_.x, 0.01f);
	int eCount = static_cast<int>(emitCount_);
	ImGui::InputInt("Emit Count", &eCount, 0, 1000);
	emitCount_ = static_cast<uint32_t>(eCount);
	if (ImGui::Button("Add Particle"))
	{
		ParticleSystem::GetInstance()->Emit(emitPosition_, emitCount_);
	}*/

	ImGui::Text("mouse position (x:%.1f, y:%.1f)", input_->GetMousePosition().x, input_->GetMousePosition().y);
	ImGui::Text("delta position (x:%.3f, y:%.3f)", input_->GetDeltaMousePosition().x, input_->GetDeltaMousePosition().y);
	ImGui::Text("particle count %d", ParticleSystem::GetInstance()->GetActiveParticleCount());
	bool useBillBorad = ParticleSystem::GetInstance()->GetUseBillboard();
	ImGui::Checkbox("use billboard", &useBillBorad);
	ParticleSystem::GetInstance()->UseBillboard(useBillBorad);

	ImGui::End();
	ImGuiManager::End();
#endif
#endif // _DEBUG

}
