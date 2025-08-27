#include <Windows.h>
#include <memory>
#include <format>
#include <cassert>

// MyCrassies
#include "BaseGame.h"

// Math
#include "Math/Vector2.h"
#include "Math/Vector4.h"
#include "Math/Matrix4x4.h"

// Utility
#include "Logger.h"
#include "StringUtility.h"

using namespace std;

// 後々フォルダとh用意する
struct  Transform
{
	Vector3 scale{};
	Vector3 rotate{};
	Vector3 translate{};
};

//struct TransformationMatrix
//{
//	Matrix4x4 wvp{};
//	Matrix4x4 world{};
//};
//
struct DirectionalLight
{
	Vector4 color{};
	Vector3 direction{};
	float intensity = 0.0f;
};

#pragma endregion

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int)
{

	unique_ptr<BaseGame> baseGame = make_unique<BaseGame>();
	baseGame->Initialize();
#pragma region Resources Create

	/*ComPtr<ID3D12Resource> directionalLightResource = dxCommon->CreateBufferResource(sizeof(DirectionalLight));
	DirectionalLight* directionalLightData = nullptr;
	directionalLightResource->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData));
	directionalLightData->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	directionalLightData->direction = Vector3(0.0f, -1.0f, 0.0f);
	directionalLightData->intensity = 1.0f;*/
#pragma endregion

#pragma region Resources Writing

#pragma region Trinangle
	//VertexData* vertexDataTriangle = nullptr;
	//// 書き込み先アドレス取得
	//vertexResourceTriangle->Map(0, nullptr, reinterpret_cast<void**>(&vertexDataTriangle));
	//vertexDataTriangle[0].position = { -0.5f, -0.5, 0.0f, 1.0f };		// left bottom
	//vertexDataTriangle[0].uv = { 0.0f, 1.0f };
	//vertexDataTriangle[1].position = { 0.0f, 0.5, 0.0f, 1.0f };			// top
	//vertexDataTriangle[1].uv = { 0.5f, 0.0f };
	//vertexDataTriangle[2].position = { 0.5f, -0.5, 0.0f, 1.0f };		// right bottom
	//vertexDataTriangle[2].uv = { 1.0f, 1.0f };

	//vertexDataTriangle[3].position = { -0.5f, -0.5, 0.5f, 1.0f };		// left bottom 2
	//vertexDataTriangle[3].uv = { 0.0f, 1.0f };
	//vertexDataTriangle[4].position = { 0.0f, 0.0, 0.0f, 1.0f };			// top 2
	//vertexDataTriangle[4].uv = { 0.5f, 0.0f };
	//vertexDataTriangle[5].position = { 0.5f, -0.5, -0.5f, 1.0f };		// right bottom 2
	//vertexDataTriangle[5].uv = { 1.0f, 1.0f };

	//TransformationMatrix* wvpDataTriangle = nullptr;
	//wvpResourceTriangle->Map(0, nullptr, reinterpret_cast<void**>(&wvpDataTriangle));
	//wvpDataTriangle->wvp = MakeIdentityMatrix();

	//MaterialData* materialDataTriangle = nullptr;
	//materialResourceTriangle->Map(0, nullptr, reinterpret_cast<void**>(&materialDataTriangle));
	//materialDataTriangle->Kd = Vector4(0.0f, 0.5f, 0.5f, 1.0f);
	// 
	//D3D12_VERTEX_BUFFER_VIEW vertexBufferViewTriangle{};
	//vertexBufferViewTriangle.BufferLocation = vertexResourceTriangle->GetGPUVirtualAddress();
	//vertexBufferViewTriangle.SizeInBytes = sizeof(VertexData) * 6;
	//vertexBufferViewTriangle.StrideInBytes = sizeof(VertexData);
#pragma endregion


#pragma region 変数宣言
	
	/*Transform triangleTransform = {};
	triangleTransform.scale = Vector3(1.0f, 1.0f, 1.0f);
	triangleTransform.translate.x = 100.0f;
	Matrix4x4 triangleWorldMatrix = MakeIdentityMatrix();*/

	

	
#pragma endregion

	while (!baseGame->EndRequest())
	{
		baseGame->Upate();
		baseGame->Draw();
	}
#pragma region Finalize
	baseGame->Finalize();

	/*materialResourceModel->Unmap(0, nullptr);
	wvpResourceModel->Unmap(0, nullptr);
	for (ComPtr<ID3D12Resource> resource : vertexResourceModel)
	{
		resource->Unmap(0, nullptr);
	}

	materialResourceTriangle->Unmap(0, nullptr);
	wvpResourceTriangle->Unmap(0, nullptr);
	vertexResourceTriangle->Unmap(0, nullptr);*/

#pragma endregion

	return 0;

}