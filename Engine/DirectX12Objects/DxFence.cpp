#include "DxFence.h"

#include "../Logger.h"
#include "DxDevice.h"
#include <cassert>

DxFence::~DxFence()
{
	CloseHandle(fenceEvent_);
}

void DxFence::Initialize(DxDevice* device)
{
	// Format to 0
	HRESULT hr = device->GetDevice()->CreateFence(fenceValue_, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence_));
	assert(SUCCEEDED(hr));
	Logger::Log("Create Fence\n");

	// Fence Created Check & signal setting
	fenceEvent_ = CreateEvent(NULL, FALSE, FALSE, NULL);
	assert(fenceEvent_ != nullptr);
	Logger::Log("Create Fence Event\n");
}

void DxFence::IncrementFenceValue()
{
	// Next Fence Value
	++fenceValue_;
}

void DxFence::WaitSignalToGPU()
{
	// Fence Value Check
	if (fence_->GetCompletedValue() < fenceValue_)
	{
		// Not Completed Wait Event Setting
		fence_->SetEventOnCompletion(fenceValue_, fenceEvent_);
		// Wait Event
		WaitForSingleObject(fenceEvent_, INFINITE);
	}
}

ID3D12Fence *DxFence::GetFence()
{
	return fence_.Get();
}

uint64_t DxFence::GetFenceValue()
{
	return fenceValue_;
}
