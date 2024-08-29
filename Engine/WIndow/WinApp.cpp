#include "WinApp.h"
#include <format>

#include "../../externals/imgui/imgui.h"
#include "../Logger.h"
#include "../StringUtility.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);

LRESULT WindowProcedure(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	if (ImGui_ImplWin32_WndProcHandler(hwnd, message, wparam, lparam))
	{
		return true;
	}
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
	Logger::Log(std::format("WindowOpened\n"));
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
		Logger::Log(std::format("WindowClosed\n"));
		return true;
	}
	return false;
}

void WinApp::Finalize()
{
	CloseWindow(hwnd_);
}

HWND WinApp::GetHWND()
{
	return hwnd_;
}
