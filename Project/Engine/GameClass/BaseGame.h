#pragma once

#include <memory>
#include <vector>

#include "Window/WinApp.h"
#include "TextureManager.h"
#include "2D/Sprite.h"
#include "3D/Object3d.h"
#include "ParticleSystem/ParticleEmitter.h"
#include "../Base/SabTusFramework.h"

class BaseGame : public SabTusFramework
{
public:

	struct  Transform
	{
		Vector3 scale{};
		Vector3 rotate{};
		Vector3 translate{};
	};

	~BaseGame();

	// 初期化
	void Initialize() override;
	// 終了
	//void Finalize() override;
	// 更新
	void Update() override;
	// 描画
	void Draw();
	// 終了予告
	//bool EndRequest() override;

private:

	void DebugWindow() override;


private:

	std::vector<std::pair<std::string, Object3d *>> drawObjects_;


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