#pragma once
#include <cstdint>

#include "Math/Vector3.h"

class ParticleEmitter
{
public:
	struct Transform
	{
		Vector3 scale{ 1.0f, 1.0f, 1.0f };
		Vector3 rotate{};
		Vector3 translate{};
	};

public:
	ParticleEmitter() = default;
	~ParticleEmitter() = default;

	void Initialize(const Vector3 &position, uint32_t emitCount = 1, float frequencyTime = 1.0f);
	void Update();

public:


	void SetEmitPosition(const Vector3 &position) { transform_.translate = position; }
	void SetEmitCount(uint32_t emitCount) { emitCount_ = emitCount; }

private:
	Transform transform_{};
	uint32_t emitCount_ = 1;
	float frequency = 0.0f;
	float freqeuncyTime_ = 1.0f;
};