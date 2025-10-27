#include <Windows.h>
#include <memory>

// MyCrassies
#include "GameClass/BaseGame.h"

// Utility
#include "Logger.h"
#include "StringUtility.h"
#include "FrameRateController.h"

using namespace std;

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int)
{
	unique_ptr<FrameRateController>fpsController = make_unique<FrameRateController>();
	fpsController->Initialize();
	unique_ptr<BaseGame> baseGame = make_unique<BaseGame>();
	baseGame->Initialize();

	// GameLoop
	while (!baseGame->EndRequest())
	{
		baseGame->Upate();
		baseGame->Draw();

		fpsController->Update();
	}
	baseGame->Finalize();
	return 0;
}