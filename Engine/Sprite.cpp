#include "Sprite.h"
#include "DirectX12Objects/DxDevice.h"

#include "DirectX12Objects/DirectX12ObjectsFunction.h"

Sprite::~Sprite()
{
	vertexResource_->Unmap(0,nullptr);
	colorResource_->Unmap(0,nullptr);
}

void Sprite::Initialize(DxDevice* device)
{
	vertexResource_ = DirectX12ObjectsFunction::CreataeBufferResource(device, sizeof(VertexData2D) * 6);
	colorResource_ = DirectX12ObjectsFunction::CreataeBufferResource(device, sizeof(Vector4) * 6);
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));
	colorResource_->Map(0, nullptr, reinterpret_cast<void**>(&colorData_));


	// 頂点インデックス設定
	indexResource_ = DirectX12ObjectsFunction::CreataeBufferResource(device, sizeof(uint32_t) * 6);
	indexBufferView_.BufferLocation = indexResource_->GetGPUVirtualAddress();
	indexBufferView_.SizeInBytes = sizeof(uint32_t) * 6;
	indexBufferView_.Format = DXGI_FORMAT_R32_UINT;
	indexResource_->Map(0, nullptr, reinterpret_cast<void**>(&indexData_));

	indexData_[0] = 0;
	indexData_[1] = 1;
	indexData_[2] = 2;
	indexData_[3] = 1;
	indexData_[4] = 3;
	indexData_[5] = 2;
}