#pragma once
#include <Windows.h>
#include <cstdint>
#include <string>

LRESULT CALLBACK WindowProcedure(HWND, UINT, WPARAM, LPARAM);

void Log(const std::string &message);

std::wstring ConvertToWString(const std::string &);
std::string ConvertToString(const std::wstring &);

class WinApp
{
public:

	static const int32_t kWindoWidth = 1280;
	static const int32_t kWindoHeight = 720;

	void Initialize();
	bool PoccesMessage();

	// getter
	HWND GetHWND();

private:

	HWND hwnd_{};
	WNDCLASS windClass_{};
	RECT windRect_ = {};
};