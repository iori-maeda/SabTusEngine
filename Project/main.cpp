#include <Windows.h>
#include <memory>

// MyCrassies
#include "GameClass/BaseGame.h"

// Utility
#include "Logger.h"
#include "StringUtility.h"

using namespace std;

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int)
{
	 
	unique_ptr<BaseGame> baseGame = make_unique<BaseGame>();
	baseGame->Initialize();

	// GameLoop
	while (!baseGame->EndRequest())
	{
		baseGame->Upate();
		baseGame->Draw();
	}
	baseGame->Finalize();
	return 0;
}