#pragma once

#include <d3d12.h>

#include "../ComPtr.h"
#include <cstdint>

class DxDevice;

class DxFence
{
public:
	~DxFence();
	void Initialize(DxDevice*);
	void IncrementFenceValue();
	void WaitSignalToGPU();

	ID3D12Fence* GetFence();
	uint64_t GetFenceValue();

private:
	ComPtr<ID3D12Fence> fence_ = nullptr;
	uint64_t fenceValue_ = 0;
	HANDLE fenceEvent_ = NULL;
};