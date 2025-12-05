#include "Triangle.h"

#include <cassert>

#include "DirectXCommon.h"
#include "DxCommand.h"
#include "DxDevice.h"
#include "DxShader.h"
#include "DxObjFunctions.h"
#include "TextureManager.h"
#include "Logger.h"

Triangle::~Triangle()
{
	vertexResource_->Unmap(0, nullptr);
}

void Triangle::Initialize(DirectXCommon *dxCommon, const Vector3 &position)
{
	assert(dxCommon);

	dxCommon_ = dxCommon;

	transform_.translate = position;

	vertexResource_ = dxCommon_->CreateBufferResource(sizeof(VertexData) * 3);
	vertexResource_->Map(0, nullptr, reinterpret_cast<void **>(&vertexData_));

	vertexData_[0].position = { -0.5f, -0.5, 0.0f, 1.0f };
	vertexData_[0].uv = { 0.0f, 1.0f };
	vertexData_[1].position = { 0.0f, 0.5, 0.0f, 1.0f };
	vertexData_[1].uv = { 0.5f, 0.0f };
	vertexData_[2].position = { 0.5f, -0.5, 0.0f, 1.0f };
	vertexData_[2].uv = { 1.0f, 1.0f };

	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	vertexBufferView_.SizeInBytes = static_cast<UINT>(sizeof(VertexData) * 3);
	vertexBufferView_.StrideInBytes = sizeof(VertexData);
}

void Triangle::Draw()
{
	ID3D12GraphicsCommandList *commandList = dxCommon_->GetCommand()->GetCommandList();

	commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);
}