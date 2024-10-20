#pragma once

#include <d3d12.h>

#include "../ComPtr.h"

namespace Dx12ObjFuncs
{
	ComPtr<ID3D12Resource> CreataeBufferResource(const ComPtr<ID3D12Device>&, size_t);

	ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(const ComPtr<ID3D12Device>&, D3D12_DESCRIPTOR_HEAP_TYPE, UINT, bool);
};

enum class ParamType
{
	PixelCBuffer,
	PixelTex,
	VertexCbuffer,
	VertexTex,

	SelectTypeNone = 99
};

namespace Dx12Structs
{
	struct RootParamMaterials
	{
		ID3D12Resource* resource = nullptr;
		ParamType type = ParamType::SelectTypeNone;
		int useRegister = -1;
	};


	template <typename T>
	struct CBufferResourceMaterial
	{
		ComPtr<ID3D12Resource> resource = nullptr;
		T* ptr = nullptr;
		ParamType type = ParamType::SelectTypeNone;
		int useRegister = -1;

		CBufferResourceMaterial() {};
		void Initialize(ID3D12Device* device, size_t sizeInBytes, ParamType type = ParamType::SelectTypeNone, int useRegister = -1);
		~CBufferResourceMaterial();
		void Map();
		void Unmap();
		RootParamMaterials GetParamsMaterials();
	};
}