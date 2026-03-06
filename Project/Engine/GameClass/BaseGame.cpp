#include "BaseGame.h"

#include <algorithm>

#include "DxObjFunctions.h"
#include "ImGuiManager.h"
#include "ParticleSystem/ParticleSystem.h"

void BaseGame::Initialize()
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

	mainCamera_ = std::make_unique<Camera>();
	mainCamera_->Initialize();
	mainCamera_->SetPosition(Vector3(0.0f, 5.0f, -15.0f));
	mainCamera_->SetRotation(Vector3(0.26f, 0.0f, 0.0f));

	input_ = std::make_unique<Input>();
	input_->Initialize(winApp_.get());

	ParticleSystem::GetInstance()->Initialize(dxCommon_.get());
	ParticleSystem::GetInstance()->SetCamera(mainCamera_.get());

	fpsController_ = std::make_unique<FrameRateController>();
	fpsController_->Initialize();

	lights_ = std::make_unique<Lights>();
	lights_->Initialize(dxCommon_.get(), mainCamera_.get());
	object3dCommon_->SetLights(lights_.get());
#pragma endregion

	sprite_ = std::make_unique<Sprite>();
	sprite_->Initiazlize(spriteCommon_.get(), "uvChecker.png");
	spriteTransform_.scale = Vector3(0.5f, 0.5f, 0.5f);
	spriteWorldMatrix_ = MakeIdentityMatrix();

	object3d_ = std::make_unique<Object3d>();
	//object3d_->Initialize(object3dCommon_.get(), "smoothSphere.obj");
	//object3d_->Initialize(object3dCommon_.get(), ModelManager::GetInstace().Load("Resources/Models/TamuraFiles/monkey_smooth/", "monkey_smooth.obj"));
	object3d_->Initialize(object3dCommon_.get(), ModelManager::GetInstace().Load("Resources/Models/CheckSphere/", "CheckSphere.gltf"));
	object3d_->SetCamera(mainCamera_.get());
	modelTransform_.scale = Vector3(1.0f, 1.0f, 1.0f);
	modelTransform_.rotate.y = -1.7f;
	modelWorldMatrix_ = MakeIdentityMatrix();

	object3d2_ = std::make_unique<Object3d>();
	object3d2_->Initialize(object3dCommon_.get(), ModelManager::GetInstace().Load("Resources/Models/chess_set_4k.gltf/", "chess_set_4k.gltf"));
	object3d2_->SetCamera(mainCamera_.get());
	modelTransform2_.scale = Vector3(10.0f, 10.0f, 10.0f);
	//modelTransform2_.rotate.x = -1.5f;
	object3d2_->SetTransform(modelTransform2_);
	modelWorldMatrix2_ = MakeIdentityMatrix();


	textureDataCPU2_ = TextureManager::GetInstace().Load("whiteTest.png");
	texColor_ = Vector4(1.0f, 1.0f, 1.0f, 1.0f);

	drawObjects_.resize(2);
	drawObjects_[0] = std::make_pair(object3d_->GetModelName(), object3d_.get());
	drawObjects_[1] = std::make_pair(object3d2_->GetModelName(), object3d2_.get());

	particleEmitter_ = std::make_unique<ParticleEmitter>();
	particleEmitter_->Initialize(emitPosition_, emitCount_,0.0f);
}

void BaseGame::Finalize()
{
	ImGuiManager::Finalize();
	ModelManager::GetInstace().Finalize();
	TextureManager::GetInstace().Finalize();

	ParticleSystem::GetInstance()->Finalize();

	winApp_->Finalize();
}

void BaseGame::Upate()
{

	winApp_->Update();
	fpsController_->Update();
	input_->UpdateAllDevice();

#ifdef USE_IMGUI
	ImGuiManager::Begin();
	fpsController_->DebugWindow();
	mainCamera_->DebugWindow();
	object3dCommon_->DebugWindow();
	object3d_->DebugWindow();
	object3d2_->DebugWindow();
	ImGui::Begin("GameDebugWinDow");
	ImGui::DragFloat3("Emit Position", &emitPosition_.x, 0.01f);
	int eCount = static_cast<int>(emitCount_);
	ImGui::InputInt("Emit Count", &eCount, 0, 1000);
	emitCount_ = static_cast<uint32_t>(eCount);
	if (ImGui::Button("Add Particle"))
	{
		ParticleSystem::GetInstance()->Emit(emitPosition_, emitCount_);
	}

	ImGui::Text("mouse position (x:%.1f, y:%.1f)", input_->GetMousePosition().x, input_->GetMousePosition().y);
	ImGui::Text("delta position (x:%.3f, y:%.3f)", input_->GetDeltaMousePosition().x, input_->GetDeltaMousePosition().y);
	ImGui::Text("particle count %d", ParticleSystem::GetInstance()->GetActiveParticleCount());
	bool useBillBorad = ParticleSystem::GetInstance()->GetUseBillboard();
	ImGui::Checkbox("use billboard", &useBillBorad);
	ParticleSystem::GetInstance()->UseBillboard(useBillBorad);

	ImGui::End();
	ImGuiManager::End();
#endif

	cameraTransform_ = mainCamera_->GetTransform();
	const float kCameraMoveSpeed = 0.1f;
	if (input_->PushKey(DIK_W))
	{
		cameraTransform_.translate.x += mainCamera_->GetForward().x * kCameraMoveSpeed;
		cameraTransform_.translate.z += mainCamera_->GetForward().z * kCameraMoveSpeed;
	}
	if (input_->PushKey(DIK_S))
	{
		cameraTransform_.translate.x += mainCamera_->GetForward().x * -kCameraMoveSpeed;
		cameraTransform_.translate.z += mainCamera_->GetForward().z * -kCameraMoveSpeed;
	}
	if (input_->PushKey(DIK_A))
	{
		cameraTransform_.translate.x += mainCamera_->GetRight().x * -kCameraMoveSpeed;
		cameraTransform_.translate.z += mainCamera_->GetRight().z * -kCameraMoveSpeed;
	}
	if (input_->PushKey(DIK_D))
	{
		cameraTransform_.translate.x += mainCamera_->GetRight().x * kCameraMoveSpeed;
		cameraTransform_.translate.z += mainCamera_->GetRight().z * kCameraMoveSpeed;
	}

	if(!input_->OnDebugWindow())
	{
		Vector2 mousePosition = input_->GetMousePosition();
		RECT winRect = winApp_->GetWindowRect();
		if (winRect.left <= mousePosition.x && winRect.right >= mousePosition.x
			&& winRect.top <= mousePosition.y && winRect.bottom >= mousePosition.y)
		{
			cameraTransform_.translate += mainCamera_->GetForward() * (input_->GetMouseWheel() / 100.0f);
		}
	}

	input_->SetCursorVisible(true);
	input_->SetMouseControll(true);
	if (!input_->OnDebugWindow() || WinApp::sIsCursorOverTitleBar)
	{
		if (input_->PushMouseButton(MouseButton::LEFT))
		{
			input_->SetCursorVisible(false);
			input_->SetMouseControll(false);
			Vector2 dir = input_->GetDeltaMousePosition();
			cameraTransform_.rotate.x += dir.y * 0.005f;
			cameraTransform_.rotate.y += dir.x * 0.005f;
		}
	}

	mainCamera_->SetTransform(cameraTransform_);
	mainCamera_->Update();

	lights_->Update();

	object3d_->Upadate();
	object3d2_->Upadate();

	particleEmitter_->SetEmitPosition(emitPosition_);
	particleEmitter_->SetEmitCount(emitCount_);
	particleEmitter_->Update();

	//triangle_->Update();
	ParticleSystem::GetInstance()->Update();

	std::sort(
		drawObjects_.begin(), drawObjects_.end(),
		[&](auto &a, auto &b)
		{
			Vector3 toA = a.second->GetPosition() - mainCamera_->GetPosition();
			Vector3 toB = b.second->GetPosition() - mainCamera_->GetPosition();
			return toA.Length() > toB.Length();
		});
}

void BaseGame::Draw()
{

	dxCommon_->BeginRendering();

	object3dCommon_->PreDraw();

	for (auto &obj : drawObjects_)
	{
		obj.second->Draw();
	}

	ParticleSystem::GetInstance()->Draw();

	spriteCommon_->PreDraw();

	//sprite_->Draw();

	ImGuiManager::Draw(dxCommon_.get());

	dxCommon_->EndRendering();
}

bool BaseGame::EndRequest()
{
	return winApp_->ProccesMessage() || input_->TriggerKey(DIK_ESCAPE);
}
