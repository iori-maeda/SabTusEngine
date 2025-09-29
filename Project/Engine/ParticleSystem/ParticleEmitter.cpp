#include "ParticleEmitter.h"
#include "ParticleSystem.h"

void ParticleEmitter::Initialize(const Vector3 &position, uint32_t emitCount, float frequencyTime)
{
	transform_.translate = position;
	emitCount_ = emitCount;
	freqeuncyTime_ = frequencyTime;
}

void ParticleEmitter::Update()
{
	frequency += 1.0f / 60.0f;
	if (frequency < freqeuncyTime_) { return; }

	ParticleSystem::GetInstance()->Emit(transform_.translate, emitCount_);
	frequency = 0.0f;
}