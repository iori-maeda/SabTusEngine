#include "BaseGame.h"

#include "DirectX12ObjectsFunction.h"
#include "ImGuiManager.h"

void BaseGame::Initialize()
{
#pragma region SystemVaiable
	winApp_ = std::make_unique<WinApp>();
	winApp_->Initialize();

	dxCommon_ = std::make_unique<DirectXCommon>();
	dxCommon_->Initialize(*winApp_.get());

	ImGuiManager::Initialize(winApp_.get(), dxCommon_.get());

	TextureManager::GetInstace().Initialize(dxCommon_.get());
	DxShaderCompiler::GetInstancxe().Initialize();

	ModelManager::GetInstace().Initialize(dxCommon_.get());

	spriteCommon_ = std::make_unique<SpriteCommon>();
	spriteCommon_->Initialize(dxCommon_.get());

	object3dCommon_ = std::make_unique<Object3dCommon>();
	object3dCommon_->Initialize(dxCommon_.get());

	mainCamera_ = std::make_unique<Camera>();
	mainCamera_->Initialize();
#pragma endregion

	sprite_ = std::make_unique<Sprite>();
	sprite_->Initiazlize(spriteCommon_.get(), "uvChecker.png");
	spriteTransform_.scale = Vector3(0.5f, 0.5f, 0.5f);
	spriteWorldMatrix_ = MakeIdentityMatrix();

	ModelManager::GetInstace().Load("sphere.obj");

	object3d_ = std::make_unique<Object3d>();
	object3d_->Initiazlize(object3dCommon_.get(), "axis.obj");
	object3d_->SetCamera(mainCamera_.get());
	modelTransform_.scale = Vector3(1.0f, 1.0f, 1.0f);
	modelTransform_.rotate.y = -1.7f;
	modelWorldMatrix_ = MakeIdentityMatrix();


	textureDataCPU2_ = TextureManager::GetInstace().Load("uvChecker.png");
	texColor_ = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
}

void BaseGame::Finalize()
{
	ImGuiManager::Finalize();
	ModelManager::GetInstace().Finalize();
	TextureManager::GetInstace().Finalize();

	winApp_->Finalize();
}

void BaseGame::Upate()
{
	winApp_->Update();

#ifdef USE_IMGUI
	ImGuiManager::Begin();
	ImGui::Begin("Debug");
	ImGui::DragFloat4("Tex Color", &texColor_.x, 0.001f, 0.0f, 1.0f);
	ImGui::DragFloat3("Model Rot", &modelTransform_.rotate.x, 0.01f);
	ImGui::End();

	mainCamera_->DebugWindow();
	ImGuiManager::End();
#endif
	mainCamera_->Update();

	
	object3d_->SetColor(texColor_);
	object3d_->SetTransform(modelTransform_);
	object3d_->Upadate();
}

void BaseGame::Draw()
{

	dxCommon_->BeginRendering();

	object3dCommon_->PreDraw();

	object3d_->Draw();

	spriteCommon_->PreDraw();

	sprite_->Draw();

	ImGuiManager::Draw(dxCommon_.get());

	dxCommon_->EndRendering();
}

bool BaseGame::EndRequest()
{
	return winApp_->PoccesMessage();
}
