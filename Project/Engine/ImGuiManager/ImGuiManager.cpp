#include "ImGuiManager.h"

#include "Window/WinApp.h"
#include "DirectXCommon.h"
#include "DxDevice.h"
#include "DxSwapChain.h"
#include "DxCommand.h"

void ImGuiManager::Initialize(WinApp *winApp, DirectXCommon *dxCommon)
{
	// こういうもん
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(winApp->GetHWND());
	ImGui_ImplDX12_Init(
		dxCommon->GetDxDevice()->GetDevice(),
		dxCommon->GetSwapChain()->kBufferCount,
		dxCommon->GetRTVDesc().Format, dxCommon->GetSRVDescriptorHeap(),
		dxCommon->GetSRVDescriptorCPUHandle(0),
		dxCommon->GetSRVDescriptorGPUHandle(0)
	);
}

void ImGuiManager::Finalize()
{
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void ImGuiManager::BeginFrame()
{
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void ImGuiManager::EndFrame(DirectXCommon *dxCommon)
{
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), dxCommon->GetCommand()->GetCommandList());
}
 