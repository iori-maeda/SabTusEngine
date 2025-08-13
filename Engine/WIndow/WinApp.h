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

	void Initialize(const std::string& title = "SubTusEngine");
	bool PoccesMessage();
	void Update();
	void Finalize();

	// getter
	HWND GetHWND()const;
	RECT GetWindowRect();

private:

	HWND hwnd_{};
	WNDCLASS windClass_{};
	RECT windRect_ = {};
};