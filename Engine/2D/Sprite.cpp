#include "Sprite.h"
#include "SpriteCommon.h"
#include "TextureManager.h"
#include "WIndow/WinApp.h"
#include "DxCommand.h"

void Sprite::Initiazlize(SpriteCommon* spriteCommon, const std::string& fileName)
{
	spriteCommon_ = spriteCommon;

	TextureDataCPU textureDataCPU = TextureManager::GetInstace().Load(fileName);
	texSize_ = { static_cast<float>(textureDataCPU.metaData.width), static_cast<float>(textureDataCPU.metaData.height) };
	texHandle_ = TextureManager::GetInstace().GetSRVDescriptorGPUHandle(textureDataCPU.fileName);

	vertexResource_ = spriteCommon_->GetDirectXCommon()->CreateBufferResource(sizeof(VertexData) * 4);
	materialResource_ = spriteCommon_->GetDirectXCommon()->CreateBufferResource(sizeof(MaterialData));
	transformationMatrixResource_ = spriteCommon_->GetDirectXCommon()->CreateBufferResource(sizeof(TransformationMatrix));


	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	vertexBufferView_.SizeInBytes = sizeof(VertexData) * 4;
	vertexBufferView_.StrideInBytes = sizeof(VertexData);

	indexResource_ = spriteCommon_->GetDirectXCommon()->CreateBufferResource(sizeof(uint32_t) * 6);
	indexBufferView_.BufferLocation = indexResource_->GetGPUVirtualAddress();
	indexBufferView_.SizeInBytes = sizeof(uint32_t) * 6;
	indexBufferView_.Format = DXGI_FORMAT_R32_UINT;

	// vertex
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));
	// left bottom
	vertexData_[0].position = { 0.0f, static_cast<float>(texSize_.x), 0.0f, 1.0f };
	vertexData_[0].uv = { 0.0f, 1.0f };
	// left top
	vertexData_[1].position = { 0.0f, 0.0f, 0.0f, 1.0f };
	vertexData_[1].uv = { 0.0f, 0.0f };
	// right bottom
	vertexData_[2].position = { static_cast<float>(texSize_.x), static_cast<float>(texSize_.y), 0.0f, 1.0f };
	vertexData_[2].uv = { 1.0f, 1.0f };
	// right top
	vertexData_[3].position = { static_cast<float>(texSize_.x), 0.0f, 0.0f, 1.0f };
	vertexData_[3].uv = { 1.0f, 0.0f };

	// index
	indexResource_->Map(0, nullptr, reinterpret_cast<void**>(&indexData_));
	indexData_[0] = 0;	// left bottom
	indexData_[1] = 1;	// left top
	indexData_[2] = 2;	// right bottom
	indexData_[3] = 1;	// left top
	indexData_[4] = 3;	// right top
	indexData_[5] = 2;	// right bottom

	transformationMatrixResource_->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData_));
	Matrix4x4 viewMatrix2D = MakeIdentityMatrix();
	Matrix4x4 projectionMatrix2D = MakeOrthoGraphicsMatrix(0.0f, 0.0f, static_cast<float>(WinApp::kWindoWidth), static_cast<float>(WinApp::kWindoHeight), 0.0f, 100.0f);

	transformationMatrixData_->world = MakeAffineMatrix(Vector3(scale_.x, scale_.y, 1.0f), Vector3(0.0f, 0.0f, rotation_), Vector3(position_.x, position_.y, 0.0f));
	transformationMatrixData_->wvp = transformationMatrixData_->world * viewMatrix2D * projectionMatrix2D;

	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
	materialData_->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
}

void Sprite::Upadate()
{
	// left bottom
	vertexData_[0].position = { 0.0f, static_cast<float>(texSize_.y), 0.0f, 1.0f };
	vertexData_[0].uv = { 0.0f, 1.0f };
	// left top
	vertexData_[1].position = { 0.0f, 0.0f, 0.0f, 1.0f };
	vertexData_[1].uv = { 0.0f, 0.0f };
	// right bottom
	vertexData_[2].position = { static_cast<float>(texSize_.x), static_cast<float>(texSize_.y), 0.0f, 1.0f };
	vertexData_[2].uv = { 1.0f, 1.0f };
	// right top
	vertexData_[3].position = { static_cast<float>(texSize_.x), 0.0f, 0.0f, 1.0f };
	vertexData_[3].uv = { 1.0f, 0.0f };

	Matrix4x4 viewMatrix2D = MakeIdentityMatrix();
	Matrix4x4 projectionMatrix2D = MakeOrthoGraphicsMatrix(0.0f, 0.0f, static_cast<float>(WinApp::kWindoWidth), static_cast<float>(WinApp::kWindoHeight), 0.0f, 100.0f);

	transformationMatrixData_->world = MakeAffineMatrix(Vector3(scale_.x, scale_.y, 1.0f), Vector3(0.0f, 0.0f, rotation_), Vector3(position_.x, position_.y, 0.0f));
	transformationMatrixData_->wvp = transformationMatrixData_->world * viewMatrix2D * projectionMatrix2D;
}

void Sprite::Draw()
{
	spriteCommon_->GetDirectXCommon()->GetCommand()->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView_);
	spriteCommon_->GetDirectXCommon()->GetCommand()->GetCommandList()->IASetIndexBuffer(&indexBufferView_);
	// CBuffer Set
	spriteCommon_->GetDirectXCommon()->GetCommand()->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformationMatrixResource_->GetGPUVirtualAddress());
	spriteCommon_->GetDirectXCommon()->GetCommand()->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());
	spriteCommon_->GetDirectXCommon()->GetCommand()->GetCommandList()->SetGraphicsRootDescriptorTable(2, texHandle_);
	// いざ描画
	spriteCommon_->GetDirectXCommon()->GetCommand()->GetCommandList()->DrawIndexedInstanced(6, 1, 0, 0, 0);
}