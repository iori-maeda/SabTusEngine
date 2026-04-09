#pragma once
#include <chrono>
#include <deque>

class FrameRateController
{
public:
	static float sTargetFrame;

public:
	~FrameRateController();
	void Initialize();
	void Update();
	void DebugWindow();

private:
	void FPSCounter();

private:

	std::chrono::steady_clock::time_point lastTime_;
	std::deque<float> deltaTimes_;
	float fps_ = 0.0f;
};

