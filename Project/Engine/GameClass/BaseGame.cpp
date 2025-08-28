#include "BaseGame.h"

#include "DirectX12ObjectsFunction.h"
#include "ImGuiManager.h"

void BaseGame::Initialize()
{
#pragma region SystemVaiable
	winApp = std::make_unique<WinApp>();
	winApp->Initialize();

	dxCommon = std::make_unique<DirectXCommon>();
	dxCommon->Initialize(*winApp.get());

	ImGuiManager::Initialize(winApp.get(), dxCommon.get());

	TextureManager::GetInstace().Initialize(dxCommon.get());
	DxShaderCompiler::GetInstancxe().Initialize();

	ModelManager::GetInstace().Initialize(dxCommon.get());

	spriteCommon = std::make_unique<SpriteCommon>();
	spriteCommon->Initialize(dxCommon.get());

	object3dCommon = std::make_unique<Object3dCommon>();
	object3dCommon->Initialize(dxCommon.get());
#pragma endregion

	sprite = std::make_unique<Sprite>();
	sprite->Initiazlize(spriteCommon.get(), "uvChecker.png");
	spriteTransform.scale = Vector3(0.5f, 0.5f, 0.5f);
	spriteWorldMatrix = MakeIdentityMatrix();

	ModelManager::GetInstace().Load("sphere.obj");

	object3d = std::make_unique<Object3d>();
	object3d->Initiazlize(object3dCommon.get(), "axis.obj");
	modelTransform.scale = Vector3(1.0f, 1.0f, 1.0f);
	modelTransform.rotate.y = -1.7f;
	modelWorldMatrix = MakeIdentityMatrix();


	textureDataCPU2 = TextureManager::GetInstace().Load("uvChecker.png");
	texColor = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
}

void BaseGame::Finalize()
{
	ImGuiManager::Finalize();
	ModelManager::GetInstace().Finalize();
	TextureManager::GetInstace().Finalize();

	winApp->Finalize();
}

void BaseGame::Upate()
{
	winApp->Update();

#ifdef USE_IMGUI

	ImGuiManager::Begin();
	ImGui::Begin("Debug");
	ImGui::DragFloat4("Tex Color", &texColor.x, 0.001f, 0.0f, 1.0f);
	ImGui::DragFloat3("Model Rot", &modelTransform.rotate.x, 0.01f);
	ImGui::End();
	ImGuiManager::End();
#endif

	
	
	object3d->SetColor(texColor);
	object3d->SetTransform(modelTransform);
	object3d->Upadate();
}

void BaseGame::Draw()
{

	dxCommon->BeginRendering();

	object3dCommon->PreDraw();

	object3d->Draw();

	spriteCommon->PreDraw();

	sprite->Draw();

	ImGuiManager::Draw(dxCommon.get());

	dxCommon->EndRendering();
}

bool BaseGame::EndRequest()
{
	return winApp->PoccesMessage();
}
