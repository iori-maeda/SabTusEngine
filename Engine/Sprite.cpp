#include "Sprite.h"
#include "DirectX12Objects/DxDevice.h"
#include "TextureManager.h"

#include "DirectX12Objects/DirectX12ObjectsFunction.h"

Sprite::~Sprite()
{
	vertexResource_->Unmap(0, nullptr);
	colorResource_->Unmap(0, nullptr);
}

void Sprite::Initialize(DxDevice *device)
{
	vertexResource_ = Dx12ObjFuncs::CreataeBufferResource(device, sizeof(VertexData2D) * 6);
	colorResource_ = Dx12ObjFuncs::CreataeBufferResource(device, sizeof(Vector4) * 6);
	transform2DResource_ = Dx12ObjFuncs::CreataeBufferResource(device, sizeof(Transform2D));
	vertexResource_->Map(0, nullptr, reinterpret_cast<void **>(&vertex_));
	colorResource_->Map(0, nullptr, reinterpret_cast<void **>(&color_));
	transform2DResource_->Map(0, nullptr, reinterpret_cast<void **>(&transform2D_));


	// 頂点インデックス設定
	indexResource_ = Dx12ObjFuncs::CreataeBufferResource(device, sizeof(uint32_t) * 6);
	indexBufferView_.BufferLocation = indexResource_->GetGPUVirtualAddress();
	indexBufferView_.SizeInBytes = sizeof(uint32_t) * 6;
	indexBufferView_.Format = DXGI_FORMAT_R32_UINT;
	indexResource_->Map(0, nullptr, reinterpret_cast<void **>(&indexData_));

	indexData_[0] = 0, indexData_[1] = 1, indexData_[2] = 2;
	indexData_[3] = 1, indexData_[4] = 3, indexData_[5] = 2;
}

void Sprite::DrawSetup(const TextureData &tex)
{
	vertex_[0].position = { 0.0f, static_cast<float>(tex.metaData.height), 0.0f, 1.0f };								// left bottom
	vertex_[0].uv = { 0.0f, 1.0f };
	vertex_[1].position = { 0.0f, 0.0f, 0.0f, 1.0f };																// left top
	vertex_[1].uv = { 0.0f, 0.0f };
	vertex_[2].position = { static_cast<float>(tex.metaData.width), static_cast<float>(tex.metaData.height), 0.0f, 1.0f };	// right bottom
	vertex_[2].uv = { 1.0f, 1.0f };

	vertex_[3].position = { 0.0f, 0.0f, 0.0f, 1.0f };																// left top
	vertex_[3].uv = { 0.0f, 0.0f };
	vertex_[4].position = { static_cast<float>(tex.metaData.width), 0.0f, 0.0f, 1.0f };								// right top
	vertex_[4].uv = { 1.0f, 0.0f };
	vertex_[5].position = { static_cast<float>(tex.metaData.width), static_cast<float>(tex.metaData.height), 0.0f, 1.0f };	// right bottom	2
	vertex_[5].uv = { 1.0f, 1.0f };
}
