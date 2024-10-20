#pragma once

#include <d3d12.h>

#include "../ComPtr.h"
#include "../Logger.h"
#include <cassert>

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
		void Initialize(ID3D12Device* device, size_t sizeInBytes, ParamType paramType = ParamType::SelectTypeNone, int useRegisterIndex = -1)
		{
			this->type = paramType;
			this->useRegister = useRegisterIndex;
			if (device == nullptr)
			{
				Logger::Log("resource not Created. device is null\n");
				return;
			}
			resource = Dx12ObjFuncs::CreataeBufferResource(device, sizeInBytes);
		}
		~CBufferResourceMaterial()
		{
			Unmap();
			resource.Reset();
			resource = nullptr;
		}
		void Map()
		{
			assert(resource != nullptr);
			if (ptr != nullptr) { return; }
			resource->Map(0, nullptr, reinterpret_cast<void**> (&ptr));
		}
		void Unmap()
		{
			if (ptr == nullptr) { return; }
			resource->Unmap(0, nullptr);
			ptr = nullptr;
		}
		RootParamMaterials GetParamsMaterials()
		{
			//assert(resource != nullptr);
			assert(type != ParamType::SelectTypeNone);
			assert(useRegister != -1);

			RootParamMaterials result{};
			result.resource = resource == nullptr ? nullptr : resource.Get();
			result.type = type;
			result.useRegister = useRegister;
			return result;
		}
	};
}