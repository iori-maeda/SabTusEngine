#pragma once
#include <random>

class Random
{
public:
	static void Initialize();
	static int GetRandom(int min = 0, int max = 1);
	static float GetRandom(float min = 0.0f, float max = 1.0f);
	static bool GetRandom();


private:
	static std::mt19937_64 engine_;
};

