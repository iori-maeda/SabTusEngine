#pragma once

#include <Windows.h>
#include <memory>
#include <format>
#include <cassert>
#include <functional>
#include <vector>

#include "ComPtr.h"
#include "Window/WinApp.h"
#include "DxDevice.h"
#include "DxCommand.h"
#include "DxSwapChain.h"
#include "DxFence.h"
#include "DxShader.h"
#include "DirectXCommon.h"
#include "TextureManager.h"
#include "2D/SpriteCommon.h"
#include "2D/Sprite.h"
#include "3D/Object3dCommon.h"
#include "3D/Object3d.h"
#include "ModelManager.h"
#include "Camera/Camera.h"
#include "IO/Input.h"
//#include "BasicShapes/Triangle.h"
#include "ParticleSystem/ParticleEmitter.h"

class BaseGame
{
public:

	struct  Transform
	{
		Vector3 scale{};
		Vector3 rotate{};
		Vector3 translate{};
	};

	// 初期化
	void Initialize();
	// 終了
	void Finalize();
	// 更新
	void Upate();
	// 描画
	void Draw();
	// 終了予告
	bool EndRequest();


private:
#pragma region SystemVaiable
	std::unique_ptr<WinApp> winApp_ = nullptr;

	std::unique_ptr<DirectXCommon> dxCommon_ = nullptr;

	std::unique_ptr<SpriteCommon> spriteCommon_ = nullptr;

	std::unique_ptr<Object3dCommon> object3dCommon_ = nullptr;

	std::unique_ptr<Camera> mainCamera_ = nullptr;

	std::unique_ptr<Input> input_ = nullptr;

	Camera::Transform cameraTransform_ = { .scale{1.0f,1.0f,1.0f},.rotate{}, .translate{} };
#pragma endregion

	std::vector<std::pair<std::string, Object3d*>> drawObjects_;


	std::unique_ptr<Sprite> sprite_ = nullptr;
	Transform spriteTransform_{};
	Matrix4x4 spriteWorldMatrix_{};

	std::unique_ptr<Object3d> object3d_;
	Object3d::Transform modelTransform_{};
	Matrix4x4 modelWorldMatrix_{};

	std::unique_ptr<Object3d> object3d2_ = nullptr;
	Model::Transform modelTransform2_{};
	Matrix4x4 modelWorldMatrix2_{};

	TextureDataCPU textureDataCPU2_;
	Vector4 texColor_{};

	Vector3 emitPosition_{};
	uint32_t emitCount_ = 1;
	std::unique_ptr<ParticleEmitter> particleEmitter_ = nullptr;
};