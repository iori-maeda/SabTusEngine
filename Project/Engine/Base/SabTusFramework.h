#pragma once
#include <memory>
#include "WinApp.h"
#include "DirectXCommon.h"
#include "2D/SpriteCommon.h"
#include "3D/Object3dCommon.h"
#include "Camera/Camera.h"
#include "IO/Input.h"
#include "FrameRateController.h"
#include "3D/Lights.h"

class SabTusFramework
{
	void Initialize();
	void Update();
	void Finalize();

	bool EndRequest();

private:
	void DebugWindow();


private:
#pragma region SystemVaiable
	std::unique_ptr<WinApp> winApp_ = nullptr;
	std::unique_ptr<DirectXCommon> dxCommon_ = nullptr;
	std::unique_ptr<SpriteCommon> spriteCommon_ = nullptr;
	std::unique_ptr<Object3dCommon> object3dCommon_ = nullptr;
	std::unique_ptr<Camera> mainCamera_ = nullptr;
	std::unique_ptr<Input> input_ = nullptr;
	std::unique_ptr<FrameRateController>fpsController_ = nullptr;
	std::unique_ptr<Lights> lights_ = nullptr;

	Vector2 clickPosition_ = Vector2();
#pragma endregion

};

