#include "FrameRateController.h"
#include <thread>
#include "ImGuiManager.h"

float FrameRateController::sTargetFrame = 60.0f;

void FrameRateController::Initialize()
{
	// 現在時刻の記録
	lastTime_ = std::chrono::steady_clock::now();
}

void FrameRateController::Update()
{
	// 1/n秒の時間
	const std::chrono::microseconds kMinTime(static_cast<uint64_t>(1000000.0f / sTargetFrame));
	// 1/nの猶予時間
	const std::chrono::microseconds kMinCheckTime(static_cast<uint64_t>(1000000.0f / (sTargetFrame + 5.0f)));

	// 現在時刻の記録
	std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();
	// 経過時間
	std::chrono::microseconds elapsed = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - lastTime_);

	DebugWindow();

	
	if (elapsed < kMinTime)
	{
		// 1/n秒の経過待ち
		while(std::chrono::steady_clock::now() - lastTime_ < kMinTime)
		{
			// スリープして待機
			std::this_thread::sleep_for(std::chrono::microseconds(1));
		}
	}
	// 現在時刻の記録
	lastTime_ = std::chrono::steady_clock::now();
}

void FrameRateController::DebugWindow()
{
#ifdef USE_IMGUI
	// 1/n秒の時間
	const std::chrono::microseconds kMinTime(static_cast<uint64_t>(1000000.0f / sTargetFrame));
	// 1/nの猶予時間
	const std::chrono::microseconds kMinCheckTime(static_cast<uint64_t>(1000000.0f / (sTargetFrame + 5.0f)));

	// 現在時刻の記録
	std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();
	// 経過時間
	std::chrono::microseconds elapsed = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - lastTime_);
	ImGui::Begin("FPS Rate");
	ImGui::Text("FPS : %.1f",  1.0f / elapsed.count());
	ImGui::End();
#endif // USE_IMGUI	
}