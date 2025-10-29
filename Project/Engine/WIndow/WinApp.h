#pragma once
#include <Windows.h>
#include <cstdint>
#include <string>

#pragma comment(lib, "winmm.lib")

LRESULT CALLBACK WindowProcedure(HWND, UINT, WPARAM, LPARAM);

class WinApp
{
public:

	static int32_t sWindoWidth;
	static int32_t sWindoHeight;
	static bool sIsCursorOverTitleBar;

public:

	void Initialize(const std::string &title = "SubTusEngine");
	bool PoccesMessage();
	void Update();
	void Finalize();

public:
	// getter
	HWND GetHWND()const;
	RECT GetWindowRect();
	HINSTANCE GetHInstance() const { return windClass_.hInstance; }

private:

	HWND hwnd_{};
	WNDCLASS windClass_{};
	RECT windRect_ = {};
};