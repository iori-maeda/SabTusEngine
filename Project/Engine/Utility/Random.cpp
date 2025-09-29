#include "Random.h"

std::mt19937_64 Random::engine_;

void Random::Initialize()
{
    std::random_device seed;
    engine_.seed(seed());
}

int Random::GetRandom(int min, int max)
{
    std::uniform_int_distribution<int> dist(min, max);
	return dist(engine_);
}

float Random::GetRandom(float min, float max)
{
    std::uniform_real_distribution<float> dist(min, max);
	return dist(engine_);
}

bool Random::GetRandom()
{
    std::bernoulli_distribution dist(0.5);
    return dist(engine_);
}
