#include "BaseGame.h"

#include <algorithm>

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
	object3d_->Initiazlize(object3dCommon_.get(), "plane.obj");
	object3d_->SetCamera(mainCamera_.get());
	modelTransform_.scale = Vector3(1.0f, 1.0f, 1.0f);
	modelTransform_.rotate.y = -1.7f;
	modelWorldMatrix_ = MakeIdentityMatrix();

	object3d2_ = std::make_unique<Object3d>();
	object3d2_->Initiazlize(object3dCommon_.get(), "smoothsphere.obj");
	object3d2_->SetCamera(mainCamera_.get());
	modelTransform2_.scale = Vector3(1.0f, 1.0f, 1.0f);
	modelTransform2_.rotate.y = -1.7f;
	modelWorldMatrix2_ = MakeIdentityMatrix();


	textureDataCPU2_ = TextureManager::GetInstace().Load("uvChecker.png");
	texColor_ = Vector4(1.0f, 1.0f, 1.0f, 1.0f);

	drawObjects_.resize(2);
	drawObjects_[0] = std::make_pair(object3d_->GetModelName(), object3d_.get());
	drawObjects_[1] = std::make_pair(object3d2_->GetModelName(), object3d2_.get());
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
	mainCamera_->DebugWindow();
	object3dCommon_->DebugWindow();
	object3d_->DebugWindow();
	object3d2_->DebugWindow();
	ImGuiManager::End();
#endif
	mainCamera_->Update();

	object3d_->Upadate();
	object3d2_->Upadate();

	std::sort(
		drawObjects_.begin(), drawObjects_.end(),
		[&](auto& a, auto& b)
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
	/*object3d_->Draw();
	object3d2_->Draw();*/
	for (auto& obj : drawObjects_)
	{
		obj.second->Draw();
	}

	spriteCommon_->PreDraw();

	//sprite_->Draw();

	ImGuiManager::Draw(dxCommon_.get());

	dxCommon_->EndRendering();
}

bool BaseGame::EndRequest()
{
	return winApp_->PoccesMessage();
}
