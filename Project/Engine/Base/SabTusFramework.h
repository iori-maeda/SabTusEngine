#pragma once
#include <memory>
#include "Math/Vector2.h"

class WinApp;
class DirectXCommon;
class SpriteCommon;
class Object3dCommon;
class Camera;
class Input;
class FrameRateController;
class Lights;

class SabTusFramework
{
public:
	~SabTusFramework();
	virtual void Initialize();
	virtual void Update();
	virtual void Finalize();

	virtual bool EndRequest();

protected:
	virtual void DebugWindow();


protected:
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

