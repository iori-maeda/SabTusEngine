#include "WinApp.h"
#include <format>

LRESULT WindowProcedure(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	default:
		break;
	}
	return DefWindowProc(hwnd, message, wparam, lparam);
}

void Log(const std::string &message)
{
	OutputDebugStringA(message.c_str());
}

std::wstring ConvertToWString(const std::string &str)
{
	if (str.empty())
	{
		return std::wstring();
	}

	auto sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char *>(&str[0]), static_cast<int>(str.size()), NULL, 0);
	if (sizeNeeded == 0)
	{
		return std::wstring();
	}

	std::wstring result(sizeNeeded, 0);
	MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char *>(&str[0]), static_cast<int>(str.size()), &result[0], sizeNeeded);

	return result;

}

std::string ConvertToString(const std::wstring &str)
{
	if (str.empty())
	{
		return std::string();
	}

	auto sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), NULL, 0, NULL, NULL);
	if (sizeNeeded == 0)
	{
		return std::string();
	}

	std::string result(sizeNeeded, 0);
	WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), result.data(), sizeNeeded, NULL, NULL);

	return result;

}

void WinApp::Initialize()
{
	// 使用するプロシージャ
	windClass_.lpfnWndProc = WindowProcedure;
	windClass_.lpszClassName = L"WindowClass";
	windClass_.hInstance = GetModuleHandle(nullptr);
	windClass_.hCursor = LoadCursor(nullptr, IDC_ARROW);

	RegisterClass(&windClass_);

	windRect_ = { 0,0,kWindoWidth,kWindoWidth };

	hwnd_ = CreateWindow(
		windClass_.lpszClassName,
		L"SabTusEngine",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		windRect_.right - windRect_.left,
		windRect_.bottom - windRect_.top,
		nullptr,
		nullptr,
		windClass_.hInstance,
		nullptr
	);
	ShowWindow(hwnd_, SW_SHOW);
	Log(std::format("WindowOpened\n"));
}

bool WinApp::PoccesMessage()
{
	MSG msg{};

	if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	if (msg.message == WM_QUIT) {
		Log(std::format("WindowClosed\n"));
		return true;
	}
	return false;
}

HWND WinApp::GetHWND()
{
	return hwnd_;
}
