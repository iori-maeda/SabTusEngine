#pragma once
#include <d3d12.h>
#include <cstdint>
#include <string>
#include <vector>
#include <memory>

#include "ComPtr.h"
#include "Math/Vector4.h"
#include "Math/Vector3.h"
#include "Math/Vector2.h"
#include "Math/Matrix4x4.h"

class DirectXCommon;

class Model
{
public:
	static std::string defaultDirectoryPath;


	struct VertexData
	{
		Vector4 position{};
		Vector2 uv{};
		Vector3 normal{};
	};

	struct MaterialData
	{
		Vector4 Ka{};
		Vector4 Kd{};
		Vector4 Ks{};
		int32_t enableLighting = true;
	};

	struct MtlData
	{
		MaterialData material{};
		std::string textureFilePath;
	};

	struct ObjectDataCPU
	{
		std::string name;
		std::vector<VertexData> vertices;
		MaterialData material{};
		std::string textureFilePath;
		VertexData* vertexData = nullptr;
		MaterialData* materialData = nullptr;
	};

	struct ObjectDataGPU
	{
		std::string name;
		ComPtr<ID3D12Resource> vertexResource = nullptr;
		ComPtr<ID3D12Resource> materialResource = nullptr;
		D3D12_GPU_DESCRIPTOR_HANDLE texHandle_{};
		Vector2 texSize_{};
		D3D12_VERTEX_BUFFER_VIEW vertexBufferViews_{};
	};

	struct ModelData
	{
		std::vector<std::pair<ObjectDataCPU, ObjectDataGPU>> objects;
	};

public:

	void Initialize(DirectXCommon* dxCommon);
	void Initialize(DirectXCommon* dxCommon, const std::string& fileName);
	void Draw();


	static ModelData LoadObjFile(const std::string& directoryPath, const std::string& filePath);
	static MtlData LoadMtlFile(const std::string& fileName, const std::string& useMaterialName);

public:

	void SetColor(const Vector4& color)
	{
		for (auto& object : modelData_.objects)
		{
			ObjectDataCPU& objCPU = object.first;
			objCPU.material.Kd = color;
			if (objCPU.materialData)
			{
				*objCPU.materialData = objCPU.material;
			}
		}
	}

private:
	DirectXCommon* dxCommon_ = nullptr;

	ModelData modelData_{};
};

