#pragma once
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"

class WinApp;
class DirectXCommon;

class ImGuiManager
{
public:
	static void Initialize(WinApp* winApp, DirectXCommon* dxCommon);
	static void Finalize();
	static void BeginFrame();
	static void EndFrame(DirectXCommon* dxCommon);
};