#pragma once
#include <Windows.h>
#include <cstdint>

LRESULT CALLBACK WindowProcedure(HWND, UINT, WPARAM, LPARAM);

class WinApp
{
private:

	WinApp();

private:

	static const int32_t kWindoWidth = 1280;
	static const int32_t kWindoHeight = 720;

	WNDCLASS windClass_{};
	RECT windRect = {};
};

