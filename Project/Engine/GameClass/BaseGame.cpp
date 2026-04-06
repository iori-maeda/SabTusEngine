#include "BaseGame.h"

#include <algorithm>

#include "DxObjFunctions.h"
#include "ImGuiManager.h"
#include "ParticleSystem/ParticleSystem.h"

void BaseGame::Initialize()
{


	sprite_ = std::make_unique<Sprite>();
	sprite_->Initiazlize(spriteCommon_.get(), "uvChecker.png");
	spriteTransform_.scale = Vector3(0.5f, 0.5f, 0.5f);
	spriteWorldMatrix_ = MakeIdentityMatrix();

	object3d_ = std::make_unique<Object3d>();
	//object3d_->Initialize(object3dCommon_.get(), "smoothSphere.obj");
	//object3d_->Initialize(object3dCommon_.get(), ModelManager::GetInstace().Load("Resources/Models/TamuraFiles/monkey/", "monkey.obj"));
	//object3d_->Initialize(object3dCommon_.get(), ModelManager::GetInstace().Load("Resources/Models/TamuraFiles/base/", "base.gltf"));
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
	particleEmitter_->Initialize(emitPosition_, emitCount_, 0.0f);
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

	

	object3d_->Upadate();
	object3d2_->Upadate();

	particleEmitter_->SetEmitPosition(emitPosition_);
	particleEmitter_->SetEmitCount(emitCount_);
	particleEmitter_->Update();

	//triangle_->Update();
	ParticleSystem::GetInstance()->Update();

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

	for (auto& obj : drawObjects_)
	{
		obj.second->Draw();
	}

	ParticleSystem::GetInstance()->Draw();

	spriteCommon_->PreDraw();

	//sprite_->Draw();

	ImGuiManager::Draw(dxCommon_.get());

	dxCommon_->EndRendering();
}


void BaseGame::DebugWindow()
{
#ifdef _DEBUG
#ifdef USE_IMGUI
	ImGuiManager::Begin();
	
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

	ImGui::End();
	ImGuiManager::End();
#endif
#endif // _DEBUG

}
