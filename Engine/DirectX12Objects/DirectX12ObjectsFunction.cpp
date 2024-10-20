#include "DirectX12ObjectsFunction.h"

#include <cassert>

#include "../Logger.h"

namespace Dx12ObjFuncs
{
	/// <summary>
	/// リソース作成関数
	/// </summary>
	/// <param name="device->GetDevice()">作成してくれるデバイス</param>
	/// <param name="sizeInBytes">使用サイズ</param>
	/// <returns></returns>
	ComPtr<ID3D12Resource> CreataeBufferResource(const ComPtr<ID3D12Device>& device, size_t sizeInBytes)
	{
		D3D12_HEAP_PROPERTIES uploadHeapPoperties{};
		uploadHeapPoperties.Type = D3D12_HEAP_TYPE_UPLOAD;
		D3D12_RESOURCE_DESC resourceDesc{};
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		resourceDesc.Width = sizeInBytes;
		/// バッファのここはテンプレ
		resourceDesc.Height = 1;
		resourceDesc.DepthOrArraySize = 1;
		resourceDesc.MipLevels = 1;
		resourceDesc.SampleDesc.Count = 1;
		resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		/// ここまでテンプレ
		ComPtr<ID3D12Resource> resource = nullptr;
		HRESULT hr = device->CreateCommittedResource(&uploadHeapPoperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&resource));
		assert(SUCCEEDED(hr));
		Logger::Log("Created Resource\n");
		return resource;
	}

	/// <summary>
	/// デスクリプタヒープ作成関数
	/// </summary>
	/// <param name="device">作成してくれるデバイス</param>
	/// <param name="heapType">作成するタイプ</param>
	/// <param name="numDescriptors">デスクリプタ個数</param>
	/// <param name="shaderVisible"></param>
	/// <returns></returns>
	ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(const ComPtr<ID3D12Device>& device, D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible)
	{
		ComPtr<ID3D12DescriptorHeap> descriptorHeap = nullptr;
		D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
		descriptorHeapDesc.Type = heapType;
		descriptorHeapDesc.NumDescriptors = numDescriptors;
		descriptorHeapDesc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		HRESULT hr = device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&descriptorHeap));
		// ディスクリプタヒープ生成確認
		assert(SUCCEEDED(hr));
		Logger::Log("CreateDecritorHeap\n");
		return descriptorHeap;
	}
}

namespace Dx12Structs
{
	template<typename T>
	void CBufferResourceMaterial<T>::Initialize(ID3D12Device* device, size_t sizeInBytes, ParamType type, int useRegister)
	{
		this->type = type;
		this->useRegister = useRegister;
		if (device == nullptr)
		{
			Logger::Log("resource not Created. device is null\n");
			return;
		}
		resource = Dx12ObjFuncs::CreataeBufferResource(device, sizeInBytes);
	}
	template<typename T>
	CBufferResourceMaterial<T>::~CBufferResourceMaterial()
	{
		Unmap();
	}
	template<typename T>
	void CBufferResourceMaterial<T>::Map()
	{
		assert(resource != nullptr);
		if (ptr != nullptr) { return; }
		resource->Map(0, nullptr, reinterpret_cast<void**> (&ptr));
	}
	template<typename T>
	void CBufferResourceMaterial<T>::Unmap()
	{
		assert(resource != nullptr);
		if (ptr == nullptr) { return; }
		resource->Unmap(0, nullptr);
	}
	template<typename T>
	RootParamMaterials CBufferResourceMaterial<T>::GetParamsMaterials()
	{
		assert(resource != nullptr);
		assert(type != ParamType::SelectTypeNone);
		assert(useRegister != -1);

		RootParamMaterials result{};
		result.resource = resource.Get();
		result.type = type;
		result.useRegister = useRegister;
		return result;
	}
}