#pragma once

#include <d3d12.h>

#include "../ComPtr.h"

class DxDevice;

class DxCommand
{
public:
	void Initialize(DxDevice*);
	void Close();
	void Reset();


	ID3D12CommandQueue* GetCommandQueue();
	ID3D12GraphicsCommandList* GetCommandList();

private:
	ComPtr<ID3D12CommandQueue> commandQueue_ = nullptr;
	ComPtr<ID3D12CommandAllocator> commandAllocator_ = nullptr;
	ComPtr<ID3D12GraphicsCommandList> commandList_ = nullptr;
};

