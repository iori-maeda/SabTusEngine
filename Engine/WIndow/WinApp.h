#pragma once
#include <Windows.h>
#include <cstdint>
#include <string>

LRESULT CALLBACK WindowProcedure(HWND, UINT, WPARAM, LPARAM);

class WinApp
{
public:

	static const int32_t kWindoWidth = 1280;
	static const int32_t kWindoHeight = 720;

	void Initialize();
	bool PoccesMessage();
	void Finalize();

	// getter
	HWND GetHWND();

private:

	HWND hwnd_{};
	WNDCLASS windClass_{};
	RECT windRect_ = {};
};