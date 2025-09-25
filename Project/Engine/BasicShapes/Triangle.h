#pragma once
#include <d3d12.h>

#include "ComPtr.h"
#include "Math/Vector2.h"
#include "Math/Vector3.h"
#include "Math/Vector4.h"
#include "Math/Matrix4x4.h"
#include "Camera/Camera.h"

class DirectXCommon;

class Triangle
{
public:
	struct  Transform
	{
		Vector3 scale{ 1.0f, 1.0f, 1.0f };
		Vector3 rotate{};
		Vector3 translate{};
	};

	struct VertexData
	{
		Vector4 position{};
		Vector2 uv{};
	};

	struct TransformationMatrix
	{
		Matrix4x4 wvp{};
		Matrix4x4 world{};
	};

	struct MaterialData
	{
		Vector4 color{};
	};

public:
	Triangle() = default;
	~Triangle();

	void Initialize(DirectXCommon *dxCommon, const Vector3 &position = Vector3());
	void Update();
	void Draw();

public:

	void SetCamera(Camera *camera) { camera_ = camera; }

private:

	void CreateRootSignature();
	void CreatePipelineStateObject();

private:

	DirectXCommon *dxCommon_ = nullptr;
	Camera *camera_ = nullptr;

	ComPtr<ID3D12RootSignature> rootSignature_ = nullptr;
	ComPtr<ID3D12PipelineState> pipelineStateObject_ = nullptr;

	Transform transform_{};

	ComPtr<ID3D12Resource> vertexResource_ = nullptr;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};
	VertexData *vertexData_ = nullptr;

	ComPtr<ID3D12Resource> transformationMatrixResource_ = nullptr;
	TransformationMatrix *transformationMatrixData_ = nullptr;

	ComPtr<ID3D12Resource> materialResource_ = nullptr;
	MaterialData *materialData_ = nullptr;

	D3D12_GPU_DESCRIPTOR_HANDLE texHandleGPU_{};
};

