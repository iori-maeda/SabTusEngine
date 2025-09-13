#include <Windows.h>
#include <memory>

// MyCrassies
#include "GameClass/BaseGame.h"

// Utility
#include "Logger.h"
#include "StringUtility.h"

using namespace std;

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int)
{

	unique_ptr<BaseGame> baseGame = make_unique<BaseGame>();
	baseGame->Initialize();

	// GameLoop
	while (!baseGame->EndRequest())
	{
		baseGame->Upate();
		baseGame->Draw();
	}
	baseGame->Finalize();
	return 0;
}

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
	
	/*Transform triangleTransform = {};
	triangleTransform.scale = Vector3(1.0f, 1.0f, 1.0f);
	triangleTransform.translate.x = 100.0f;
	Matrix4x4 triangleWorldMatrix = MakeIdentityMatrix();*/
#pragma endregion