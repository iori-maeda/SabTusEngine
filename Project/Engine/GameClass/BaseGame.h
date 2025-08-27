#pragma once

#include <Windows.h>
#include <memory>
#include <format>
#include <cassert>

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
	std::unique_ptr<WinApp> winApp = nullptr;

	std::unique_ptr<DirectXCommon> dxCommon = nullptr;

	std::unique_ptr<SpriteCommon> spriteCommon = nullptr;

	std::unique_ptr<Object3dCommon> object3dCommon = nullptr;
#pragma endregion


	std::unique_ptr<Sprite> sprite = nullptr;
	std::unique_ptr<Object3d> object3d = nullptr;

	TextureDataCPU textureDataCPU2;


};