#pragma once

#include <d3d12.h>

#include "../ComPtr.h"

namespace DirectX12ObjectsFunction
{
	ComPtr<ID3D12Resource> CreataeBufferResource(const ComPtr<ID3D12Device>&, size_t);
};