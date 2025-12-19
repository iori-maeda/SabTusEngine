#include "FrameRateController.h"

#include <thread>
#include <format>

#include "ImGuiManager.h"
#include "Logger.h"

using namespace std::chrono;

float FrameRateController::sTargetFrame = 60.0f;

void FrameRateController::Initialize()
{
	// 現在時刻の記録
	lastTime_ = std::chrono::steady_clock::now();

	deltaTimes_.resize(static_cast<size_t>(sTargetFrame));
}

void FrameRateController::Update()
{
	// 1/n秒の時間
	const std::chrono::microseconds kMinTime(static_cast<uint64_t>(1000000.0f / sTargetFrame));
	// 1/nの猶予時間
	//const std::chrono::microseconds kMinCheckTime(static_cast<uint64_t>(1000000.0f / (sTargetFrame + 5.0f)));
	const std::chrono::microseconds kSleepThreshold(static_cast<uint64_t>(1500));

	// 現在時刻の記録
	std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();
	// 経過時間
	std::chrono::microseconds elapsed = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - lastTime_);

	while (kSleepThreshold + elapsed < kMinTime)
	{
		std::this_thread::sleep_for(std::chrono::microseconds(500));
		elapsed = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - lastTime_);
	}

	while (elapsed < kMinTime)
	{
		elapsed = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - lastTime_);
	}

	// 経過時間を収集
	deltaTimes_.push_back(duration<float>(elapsed).count());
	// 指定したFPSを超える数なら先頭データを破棄
	if (deltaTimes_.size() > static_cast<size_t>(sTargetFrame)) { deltaTimes_.pop_front(); }

	// 現在時刻の記録
	lastTime_ = std::chrono::steady_clock::now();

	FPSCounter();
}

void FrameRateController::DebugWindow()
{
#ifdef USE_IMGUI
	ImGui::Begin("FPS Rate");
	ImGui::Text("FPS : %.1f", fps_);
	ImGui::End();
#endif // USE_IMGUI	
}

void FrameRateController::FPSCounter()
{
	if (deltaTimes_.empty()) { return; }
	float avgDeltaTime = 0.0f;
	for (float deltaTime : deltaTimes_) { avgDeltaTime += deltaTime; }
	avgDeltaTime /= deltaTimes_.size();
	if (avgDeltaTime <= 0.0f)
	{
		fps_ = -1.0f;
		return;
	}
	fps_ = 1.0f / avgDeltaTime;
	//Logger::Log(std::format("FPS Rate:{}\n", fps_));
}
