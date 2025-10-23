#include "ImGuiManager.h"

#include "Window/WinApp.h"
#include "DirectXCommon.h"
#include "DxDevice.h"
#include "DxSwapChain.h"
#include "DxCommand.h"

WinApp* ImGuiManager::winApp_ = nullptr;

void ImGuiManager::Initialize([[maybe_unused]]WinApp *winApp, [[maybe_unused]]DirectXCommon *dxCommon)
{
#ifdef USE_IMGUI
	assert(winApp != nullptr);
	winApp_ = winApp;
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
#endif // USE_IMGUI
}

void ImGuiManager::Finalize()
{
#ifdef USE_IMGUI
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
#endif
}

void ImGuiManager::Begin()
{
#ifdef USE_IMGUI
	
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
#endif
}

void ImGuiManager::End()
{
	
#ifdef USE_IMGUI
	ImGui::Render();
#endif
}

void ImGuiManager::Draw([[maybe_unused]]DirectXCommon *dxCommon)
{
#ifdef USE_IMGUI
	ID3D12DescriptorHeap *descriptorHeaps[] = { dxCommon->GetSRVDescriptorHeap() };
	dxCommon->GetCommand()->GetCommandList()->SetDescriptorHeaps(1, descriptorHeaps);
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), dxCommon->GetCommand()->GetCommandList());
#endif
}
