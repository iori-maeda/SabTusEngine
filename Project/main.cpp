#include <Windows.h>
#include <memory>
#include <format>
#include <cassert>
#include <vector>
#include <fstream>
#include <sstream>

// DirectX12
#include <d3d12.h>
#include <dxgi1_6.h>
#pragma comment(lib, "d3d12.lib")
// ShaderComplier
#include <dxcapi.h>
#pragma comment(lib, "dxcompiler.lib")
// ImGui

#include "externals/DirectXTex/DirectXTex.h"
#include "externals/DirectXTex/d3dx12.h"


// MyCrassies
#include "ComPtr.h"
#include "Window/WinApp.h"
#include "DxDevice.h"
#include "DxCommand.h"
#include "DxSwapChain.h"
#include "DxFence.h"
#include "DxShader.h"
#include "DirectXCommon.h"
#include "TextureManager.h"
#include "2D/SpriteCommon.h"
#include "2D/Sprite.h"
#include "3D/Object3dCommon.h"
#include "3D/Object3d.h"
#include "ModelManager.h"

#include "DirectX12ObjectsFunction.h"
#include "ImGuiManager.h"

// Math
#include "Math/Vector2.h"
#include "Math/Vector4.h"
#include "Math/Matrix4x4.h"

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
	unique_ptr<WinApp> winApp = make_unique<WinApp>();
	winApp->Initialize();

	unique_ptr<DirectXCommon> dxCommon = make_unique<DirectXCommon>();
	dxCommon->Initialize(*winApp.get());

	ImGuiManager::Initialize(winApp.get(), dxCommon.get());

	TextureManager::GetInstace().Initialize(dxCommon.get());
	DxShaderCompiler::GetInstancxe().Initialize();

	ModelManager::GetInstace().Initialize(dxCommon.get());

	unique_ptr<SpriteCommon> spriteCommon = make_unique<SpriteCommon>();
	spriteCommon->Initialize(dxCommon.get());

	unique_ptr<Object3dCommon> object3dCommon = make_unique<Object3dCommon>();
	object3dCommon->Initialize(dxCommon.get());

	unique_ptr<Sprite> sprite = make_unique<Sprite>();
	sprite->Initiazlize(spriteCommon.get(), "uvChecker.png");

	ModelManager::GetInstace().Load("sphere.obj");

	unique_ptr<Object3d> object3d = make_unique<Object3d>();
	object3d->Initiazlize(object3dCommon.get(), "axis.obj");


	TextureDataCPU textureDataCPU2 = TextureManager::GetInstace().Load("uvChecker.png");


#pragma region Resources Create

	ComPtr<ID3D12Resource> directionalLightResource = dxCommon->CreateBufferResource(sizeof(DirectionalLight));
	DirectionalLight* directionalLightData = nullptr;
	directionalLightResource->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData));
	directionalLightData->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	directionalLightData->direction = Vector3(0.0f, -1.0f, 0.0f);
	directionalLightData->intensity = 1.0f;
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
	Transform mainCameraTransform = {};
	mainCameraTransform.scale = Vector3(1.0f, 1.0f, 1.0f);
	mainCameraTransform.translate.z = -200.0f;
	Matrix4x4 mainCameraMatrix = MakeAffineMatrix(mainCameraTransform.scale, mainCameraTransform.rotate, mainCameraTransform.translate);
	Matrix4x4 mainCameraViewMatrix = MakeInVerse(mainCameraMatrix);
	Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(0.45f, static_cast<float>(WinApp::kWindoWidth) / static_cast<float>(WinApp::kWindoHeight), 0.1f, 100.0f);

	/*Transform triangleTransform = {};
	triangleTransform.scale = Vector3(1.0f, 1.0f, 1.0f);
	triangleTransform.translate.x = 100.0f;
	Matrix4x4 triangleWorldMatrix = MakeIdentityMatrix();*/

	Transform spriteTransform = {};
	spriteTransform.scale = Vector3(0.5f, 0.5f, 0.5f);
	Matrix4x4 spriteWorldMatrix = MakeIdentityMatrix();

	Object3d::Transform modelTransform = {};
	modelTransform.scale = Vector3(1.0f, 1.0f, 1.0f);
	modelTransform.rotate.y = -1.7f;
	Matrix4x4 modelWorldMatrix = MakeIdentityMatrix();

	Vector4 texColor = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
#pragma endregion

	while (!winApp->PoccesMessage())
	{

#ifdef USE_IMGUI

		ImGuiManager::Begin();
		ImGui::Begin("Debug");
		ImGui::DragFloat3("Main Camera Position", &mainCameraTransform.translate.x, 0.1f);
		ImGui::DragFloat4("Tex Color", &texColor.x, 0.001f, 0.0f, 1.0f);
		ImGui::DragFloat3("Model Rot", &modelTransform.rotate.x, 0.01f);
		ImGui::DragFloat3("Light Dir", &directionalLightData->direction.x, 0.01f);
		ImGui::End();
		ImGuiManager::End();
#endif

		winApp->Update();

#pragma region GameUpdate
		projectionMatrix = MakePerspectiveFovMatrix(0.45f, static_cast<float>(WinApp::kWindoWidth) / static_cast<float>(WinApp::kWindoHeight), 0.1f, 100.0f);
		//projectionMatrix2D = MakeOrthoGraphicsMatrix(0.0f, 0.0f, static_cast<float>(WinApp::kWindoWidth), static_cast<float>(WinApp::kWindoHeight), 0.0f, 100.0f);

		mainCameraMatrix = MakeAffineMatrix(mainCameraTransform.scale, mainCameraTransform.rotate, mainCameraTransform.translate);
		mainCameraViewMatrix = MakeInVerse(mainCameraMatrix);

		object3d->SetColor(texColor);
		object3d->SetTransform(modelTransform);
		object3d->Upadate();
	/*	materialDataTriangle->Kd = texColor;

		triangleTransform.rotate.y += 0.01f;
		triangleWorldMatrix = MakeAffineMatrix(triangleTransform.scale, triangleTransform.rotate, triangleTransform.translate);
		wvpDataTriangle->wvp = triangleWorldMatrix * mainCameraViewMatrix * projectionMatrix;
		wvpDataTriangle->world = triangleWorldMatrix;*/

		//modelTransform.rotate.y += 0.03f;
		/*modelWorldMatrix = MakeAffineMatrix(modelTransform.scale, modelTransform.rotate, modelTransform.translate);
		wvpDataModel->wvp = modelWorldMatrix * mainCameraViewMatrix * projectionMatrix;
		wvpDataModel->world = modelWorldMatrix;*/
#pragma endregion

		dxCommon->BeginRendering();

#pragma region 3D Draw
		object3dCommon->PreDraw();

		object3d->Draw();
#pragma endregion

#pragma region 2D Draw
		spriteCommon->PreDraw();
		sprite->Draw();
#pragma endregion

#pragma region PostDraw
#pragma region ImGui Set
		ImGuiManager::Draw(dxCommon.get());
#pragma endregion

		dxCommon->EndRendering();
	}
#pragma region Finalize


	/*materialResourceModel->Unmap(0, nullptr);
	wvpResourceModel->Unmap(0, nullptr);
	for (ComPtr<ID3D12Resource> resource : vertexResourceModel)
	{
		resource->Unmap(0, nullptr);
	}

	materialResourceTriangle->Unmap(0, nullptr);
	wvpResourceTriangle->Unmap(0, nullptr);
	vertexResourceTriangle->Unmap(0, nullptr);*/
	ImGuiManager::Finalize();
	ModelManager::GetInstace().Finalize();
	TextureManager::GetInstace().Finalize();

	winApp->Finalize();

#pragma endregion

	return 0;

}