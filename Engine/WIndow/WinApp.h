#pragma once
#include <Windows.h>
#include <cstdint>
#include <string>

LRESULT CALLBACK WindowProcedure(HWND, UINT, WPARAM, LPARAM);

class WinApp
{
public:

	static int32_t kWindoWidth;
	static int32_t kWindoHeight;

	void Initialize();
	bool PoccesMessage();
	void Update();
	void Finalize();

	// getter
	HWND GetHWND();

private:

	HWND hwnd_{};
	WNDCLASS windClass_{};
	RECT windRect_ = {};
};