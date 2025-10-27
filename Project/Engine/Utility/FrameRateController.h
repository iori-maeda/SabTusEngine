#pragma once
#include <chrono>
#include <cstdint>

class FrameRateController
{
public:
	static float sTargetFrame;

public:
	void Initialize();
	void Update();
	void DebugWindow();

private:

	std::chrono::steady_clock::time_point lastTime_;
};

