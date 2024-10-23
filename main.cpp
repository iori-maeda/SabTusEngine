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
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"

#include "externals/DirectXTex/DirectXTex.h"
#include "externals/DirectXTex/d3dx12.h"


// MyCrassies
#include "Engine/ComPtr.h"
#include "Engine/Window/WinApp.h"
#include "Engine/DirectX12Objects/DxDevice.h"
#include "Engine/DirectX12Objects/DxCommand.h"
#include "Engine/DirectX12Objects/DxSwapChain.h"
#include "Engine/DirectX12Objects/DxFence.h"
#include "Engine/DirectX12Objects/DxRootSignature.h"
#include "Engine/DirectX12Objects/DxShaderManager.h"
#include "Engine/DirectX12Objects/DxPipelineStateObject.h"
#include "Engine/TextureManager.h"

#include "Engine/DirectX12Objects/DirectX12ObjectsFunction.h"

// Math
#include "Engine/Math/Vector2.h"
#include "Engine/Math/Vector4.h"
#include "Engine/Math/Matrix4x4.h"

#include "Engine/Logger.h"
#include "Engine/StringUtility.h"

using namespace std;

// 後々フォルダとh用意する
struct  Transform
{
	Vector3 scale{};
	Vector3 rotate{};
	Vector3 translate{};
};

struct TransformationMatrix
{
	Matrix4x4 wvp{};
	Matrix4x4 world{};
};

struct DirectionalLight
{
	Vector4 color{};
	Vector3 direction{};
	float intensity = 0.0f;
};

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
	std::string textureFileName = "uvChecker.png";
};

struct ObjectData
{
	std::string name;
	std::vector<VertexData> vertices;
	MaterialData material{};
	TextureData texData{};
};

struct ModelData
{
	std::vector<ObjectData> objects;
};

#pragma region functoins
DirectX::ScratchImage LoadTexture(const std::string&);
ComPtr<ID3D12Resource> CreateTextureResource(const ComPtr<ID3D12Device>&, const DirectX::TexMetadata&);
ComPtr<ID3D12Resource> UploadTextureData(const ComPtr<ID3D12Resource>&, const DirectX::ScratchImage&, const ComPtr<ID3D12Device>&, const ComPtr<ID3D12GraphicsCommandList>&);

D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(const ComPtr<ID3D12DescriptorHeap>&, uint32_t, uint32_t);
D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(const ComPtr<ID3D12DescriptorHeap>&, uint32_t, uint32_t);

ComPtr<ID3D12Resource> CreateDepthStencilTextureResource(const ComPtr<ID3D12Device>&, int32_t, int32_t);

ModelData LoadObjFile(const std::string&, const std::string&);
MaterialData LoadMtlFile(const std::string&, const std::string&);
#pragma endregion

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int)
{
	// Texture読み込みのためCOMを初期化
	HRESULT hr;
	hr = CoInitializeEx(0, COINIT_MULTITHREADED);

	unique_ptr<WinApp> winApp = make_unique<WinApp>();
	winApp->Initialize();

	unique_ptr<DxDevice> device = make_unique<DxDevice>();
	device->Initialize();

	unique_ptr<DxCommand> command = make_unique<DxCommand>();
	command->Initialize(device.get());

	unique_ptr<DxSwapChain> swapChain = make_unique<DxSwapChain>();
	swapChain->Initialize(winApp.get(), device.get(), command.get());
	UINT backBufferIndex = swapChain->GetBackBufferIndex();

	unique_ptr<DxFence> fence = make_unique<DxFence>();
	fence->Initialize(device.get());

	unique_ptr<DxRootSignature>rootSignature = make_unique<DxRootSignature>();
#pragma region ルートパラメータ回り
	Dx12Structs::CBufferResourceMaterial<VertexData> vertex{};
	vertex.Initialize(device->GetDevice(), static_cast<size_t>(sizeof(VertexData) * 6), ParamType::VertexCbuffer, 0);
	vertex.Map();
	// ルートパラメータへ追加
	//rootSignature->AddRootParameter("vertexData", vertex.GetParamsMaterials());

	Dx12Structs::CBufferResourceMaterial<TransformationMatrix> transformationMatrix{};
	transformationMatrix.Initialize(device->GetDevice(), static_cast<size_t>(sizeof(TransformationMatrix)), ParamType::VertexCbuffer, 0);
	transformationMatrix.Map();
	// ルートパラメータへ追加
	rootSignature->AddRootParameter("transformationMatrix", transformationMatrix.GetParamsMaterials());

	Dx12Structs::CBufferResourceMaterial<MaterialData> material{};
	material.Initialize(device->GetDevice(), static_cast<size_t>(sizeof(MaterialData)), ParamType::PixelCBuffer, 0);
	material.Map();
	// ルートパラメータへ追加
	rootSignature->AddRootParameter("materialData", material.GetParamsMaterials());
	
	Dx12Structs::CBufferResourceMaterial<DirectionalLight> directionalLight{};
	directionalLight.Initialize(device->GetDevice(), static_cast<size_t>(sizeof(DirectionalLight)), ParamType::PixelCBuffer, 1);
	directionalLight.Map();
	// ルートパラメータへ追加
	rootSignature->AddRootParameter("directionalLight", directionalLight.GetParamsMaterials());

	Dx12Structs::CBufferResourceMaterial<char> texParam;
	texParam.Initialize(nullptr, 0, ParamType::PixelTex, 0);
	rootSignature->AddRootParameter("texture", texParam.GetParamsMaterials());
#pragma endregion
	rootSignature->Create(device.get());

	unique_ptr<DxShaderManager> shaderManager = make_unique<DxShaderManager>();
	shaderManager->Initialize();
	ComPtr<IDxcBlob> vertexShaderBlob = shaderManager->CompileShader("Basic3DVS.hlsl", L"vs_6_0");
	ComPtr<IDxcBlob> pixelShaderBlob = shaderManager->CompileShader("Basic3DPS.hlsl", L"ps_6_0");

	unique_ptr<DxPipelineStateObject> pipelineState = make_unique<DxPipelineStateObject>();
	pipelineState->InitializeAndCreate(device.get(), rootSignature.get(), pixelShaderBlob.Get(), vertexShaderBlob.Get());



#pragma region DescriptorHeap Create
	ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap = Dx12ObjFuncs::CreateDescriptorHeap(device->GetDevice(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 2, false);
	ComPtr<ID3D12DescriptorHeap> srvDescriptorHeap = Dx12ObjFuncs::CreateDescriptorHeap(device->GetDevice(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 128, true);
	ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap = Dx12ObjFuncs::CreateDescriptorHeap(device->GetDevice(), D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);
#pragma endregion

	unique_ptr<TextureManager> texManager = make_unique<TextureManager>();
	texManager->Initialize(device.get(), command.get(), fence.get(), srvDescriptorHeap.Get());

#pragma region RenderTargetView Create
	// RTVSettings
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;		// 出力結果をSRGBにして書き込み
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;	// 2Dテクスチャとして書き込み
	D3D12_CPU_DESCRIPTOR_HANDLE rtvStartHandle = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	// RTVを2つ作成するためディスクリプタも2つ用意
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2]{};
	// ディスクリプタの先頭に1つ目を作成
	rtvHandles[0] = rtvStartHandle;
	device->GetDevice()->CreateRenderTargetView(swapChain->GetSwapChainResourceByIndex(0), &rtvDesc, rtvHandles[0]);
	// 先頭からディスクリプタのサイズ分移動した場所のハンドルを取得
	rtvHandles[1].ptr = rtvHandles[0].ptr + device->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	// 2つ目を作成
	device->GetDevice()->CreateRenderTargetView(swapChain->GetSwapChainResourceByIndex(1), &rtvDesc, rtvHandles[1]);

#pragma endregion

#pragma region DipthStencil Resource & View Create
	ComPtr<ID3D12Resource> depthStencilResource = CreateDepthStencilTextureResource(device->GetDevice(), WinApp::kWindoWidth, WinApp::kWindoHeight);

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;	// 基本Resouceにあわせる
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	// DSVHeapの先頭に作成
	device->GetDevice()->CreateDepthStencilView(depthStencilResource.Get(), &dsvDesc, dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
#pragma endregion

	// Windor Drawing Step
	// State Present -> Render
	// Transition Barrier Set
	command->GetCommandList()->ResourceBarrier(1, swapChain->GetBarrier(D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
	backBufferIndex = swapChain->GetBackBufferIndex();

	// 描画先のRTｖをバックバッファのインデックスをもとに設定
	command->GetCommandList()->OMSetRenderTargets(1, &rtvHandles[backBufferIndex], false, nullptr);
	// 画面全体をクリア
	float clearColor[] = { 0.1f, 0.25f, 0.5f, 1.0f };
	command->GetCommandList()->ClearRenderTargetView(rtvHandles[backBufferIndex], clearColor, 0, nullptr);

	// Windor Drawing Step
	// State Render -> Present
	// Transition Barrier Set
	command->GetCommandList()->ResourceBarrier(1, swapChain->GetBarrier(D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

#pragma region GPU Work Transfer & Wait
	// CommandList Close & Kick
	command->Close();
	// GPUとOSに画面の交換を依頼を通知
	swapChain->Present(1, 0);
	// Fence Wait
	fence->IncrementFenceValue();
	command->GetCommandQueue()->Signal(fence->GetFence(), fence->GetFenceValue());
	fence->WaitSignalToGPU();
	// commandList Reset
	command->Reset();
#pragma endregion

#pragma region Model Load
	ModelData modelData = LoadObjFile("Resources/Models", "multiMaterial.obj");
#pragma endregion

#pragma region Texture
	for (ObjectData& obj : modelData.objects) {
		obj.texData = texManager->LoadTexrureData(obj.material.textureFileName, obj.name);
	}
	TextureData monsterBallTex = texManager->LoadTexrureData("monsterBall.png");
#pragma endregion

#pragma region Resources Create

	std::vector<ComPtr<ID3D12Resource>> vertexResourceModel;
	// オブジェクトの数分領域確保
	vertexResourceModel.reserve(modelData.objects.size());
	for (ObjectData& obj : modelData.objects)
	{
		// 実際に追加
		vertexResourceModel.push_back(Dx12ObjFuncs::CreataeBufferResource(device->GetDevice(), sizeof(VertexData) * obj.vertices.size()));
	}
	ComPtr<ID3D12Resource> wvpResourceModel = Dx12ObjFuncs::CreataeBufferResource(device->GetDevice(), sizeof(TransformationMatrix));
	ComPtr<ID3D12Resource> materialResourceModel = Dx12ObjFuncs::CreataeBufferResource(device->GetDevice(), sizeof(MaterialData));
#pragma endregion

#pragma region Resources Writing

#pragma region Trinangle
	VertexData* vertexDataTriangle = vertex.ptr;
	// 書き込み先アドレス取得
	//vertexResourceTriangle->Map(0, nullptr, reinterpret_cast<void**>(&vertexDataTriangle));
	vertexDataTriangle[0].position = { -0.5f, -0.5, 0.0f, 1.0f };		// left bottom
	vertexDataTriangle[0].uv = { 0.0f, 1.0f };
	vertexDataTriangle[1].position = { 0.0f, 0.5, 0.0f, 1.0f };			// top
	vertexDataTriangle[1].uv = { 0.5f, 0.0f };
	vertexDataTriangle[2].position = { 0.5f, -0.5, 0.0f, 1.0f };		// right bottom
	vertexDataTriangle[2].uv = { 1.0f, 1.0f };

	vertexDataTriangle[3].position = { -0.5f, -0.5, 0.5f, 1.0f };		// left bottom 2
	vertexDataTriangle[3].uv = { 0.0f, 1.0f };
	vertexDataTriangle[4].position = { 0.0f, 0.0, 0.0f, 1.0f };			// top 2
	vertexDataTriangle[4].uv = { 0.5f, 0.0f };
	vertexDataTriangle[5].position = { 0.5f, -0.5, -0.5f, 1.0f };		// right bottom 2
	vertexDataTriangle[5].uv = { 1.0f, 1.0f };

	TransformationMatrix* wvpDataTriangle = transformationMatrix.ptr;
	//wvpResourceTriangle->Map(0, nullptr, reinterpret_cast<void**>(&wvpDataTriangle));
	wvpDataTriangle->wvp = MakeIdentityMatrix();

	MaterialData* materialDataTriangle = material.ptr;
	//materialResourceTriangle->Map(0, nullptr, reinterpret_cast<void**>(&materialDataTriangle));
	materialDataTriangle->Kd = Vector4(0.0f, 0.5f, 0.5f, 1.0f);
#pragma endregion

#pragma region Sprite
	//VertexData* vertexDataSprite = nullptr;
	//// 書き込み先アドレス取得
	//vertexResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&vertexDataSprite));
	//vertexDataSprite[0].position = { 0.0f, static_cast<float>(monsterBallTex.metaData.height), 0.0f, 1.0f };								// left bottom
	//vertexDataSprite[0].uv = { 0.0f, 1.0f };
	//vertexDataSprite[1].position = { 0.0f, 0.0f, 0.0f, 1.0f };																// left top
	//vertexDataSprite[1].uv = { 0.0f, 0.0f };
	//vertexDataSprite[2].position = { static_cast<float>(monsterBallTex.metaData.width), static_cast<float>(monsterBallTex.metaData.height), 0.0f, 1.0f };	// right bottom
	//vertexDataSprite[2].uv = { 1.0f, 1.0f };

	//vertexDataSprite[3].position = { 0.0f, 0.0f, 0.0f, 1.0f };																// left top
	//vertexDataSprite[3].uv = { 0.0f, 0.0f };
	//vertexDataSprite[4].position = { static_cast<float>(monsterBallTex.metaData.width), 0.0f, 0.0f, 1.0f };								// right top
	//vertexDataSprite[4].uv = { 1.0f, 0.0f };
	//vertexDataSprite[5].position = { static_cast<float>(monsterBallTex.metaData.width), static_cast<float>(monsterBallTex.metaData.height), 0.0f, 1.0f };	// right bottom	2
	//vertexDataSprite[5].uv = { 1.0f, 1.0f };

	//TransformationMatrix* wvpDataSprite = nullptr;
	//wvpResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&wvpDataSprite));
	//wvpDataSprite->wvp = MakeIdentityMatrix();

	//MaterialData* materialDataSprite = nullptr;
	//materialResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&materialDataSprite));
	//materialDataSprite->Kd = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	//materialDataSprite->enableLighting = false;
#pragma endregion

#pragma region Model
	std::vector<VertexData*> vertexDataModel;
	vertexDataModel.resize(modelData.objects.size());
	for (size_t i = 0; i < modelData.objects.size(); ++i)
	{
		//vertexResourceModel[i]->Map(0, nullptr, reinterpret_cast<void**>(&vertexDataModel[i]));
		vertexDataModel[i] = vertex.ptr;
		// 頂点データコピー
		if (vertexDataModel[i] != nullptr)
		{
			std::memcpy(vertexDataModel[i], modelData.objects[i].vertices.data(), sizeof(VertexData) * modelData.objects[i].vertices.size());
		}
	}

	TransformationMatrix* wvpDataModel = transformationMatrix.ptr;
	wvpDataModel->wvp = MakeIdentityMatrix();

	MaterialData* materialDataModel = material.ptr;
	*materialDataModel = modelData.objects[0].material;
	materialDataModel->Kd = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	//materialDataModel->enableLighting = false;
#pragma endregion

	DirectionalLight* directionalLightData = directionalLight.ptr;
	directionalLightData->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	directionalLightData->direction = Vector3(0.0f, -1.0f, 0.0f);
	directionalLightData->intensity = 1.0f;

#pragma endregion

#pragma region VertexBufferView Create
	D3D12_VERTEX_BUFFER_VIEW vertexBufferViewTriangle{};
	vertexBufferViewTriangle.BufferLocation = vertex.resource->GetGPUVirtualAddress();
	vertexBufferViewTriangle.SizeInBytes = sizeof(VertexData) * 6;
	vertexBufferViewTriangle.StrideInBytes = sizeof(VertexData);

	/*D3D12_VERTEX_BUFFER_VIEW vertexBufferViewSprite{};
	vertexBufferViewSprite.BufferLocation = vertexResourceSprite->GetGPUVirtualAddress();
	vertexBufferViewSprite.SizeInBytes = sizeof(VertexData) * 6;
	vertexBufferViewSprite.StrideInBytes = sizeof(VertexData);*/

	std::vector<D3D12_VERTEX_BUFFER_VIEW>vertexBufferViewModel;
	vertexBufferViewModel.reserve(modelData.objects.size());
	for (size_t i = 0; i < modelData.objects.size(); ++i)
	{
		D3D12_VERTEX_BUFFER_VIEW bufferView{};
		bufferView.BufferLocation = vertexResourceModel[i]->GetGPUVirtualAddress();
		bufferView.SizeInBytes = static_cast<UINT>(sizeof(VertexData) * modelData.objects[i].vertices.size());
		bufferView.StrideInBytes = sizeof(VertexData);
		vertexBufferViewModel.push_back(bufferView);
	}
#pragma endregion

#pragma region Viewport & Scissor
	D3D12_VIEWPORT viewport{};
	// 画面全体に表示
	viewport.Width = static_cast<FLOAT>(WinApp::kWindoWidth);
	viewport.Height = static_cast<FLOAT>(WinApp::kWindoHeight);
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	// scissor
	D3D12_RECT scissorRect{};
	// 基本的にはviewportと同じ構成で
	scissorRect.left = 0;
	scissorRect.right = WinApp::kWindoWidth;
	scissorRect.top = 0;
	scissorRect.bottom = WinApp::kWindoHeight;
#pragma endregion

#pragma region ImGui Initialize
	// こういうもん
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(winApp->GetHWND());
	ImGui_ImplDX12_Init(
		device->GetDevice(),
		swapChain->kBufferCount,
		rtvDesc.Format, srvDescriptorHeap.Get(),
		srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
		srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
#pragma endregion

#pragma region 変数宣言
	Transform mainCameraTransform = {};
	mainCameraTransform.scale = Vector3(1.0f, 1.0f, 1.0f);
	mainCameraTransform.translate.z = -10.0f;
	Matrix4x4 mainCameraMatrix = MakeAffineMatrix(mainCameraTransform.scale, mainCameraTransform.rotate, mainCameraTransform.translate);
	Matrix4x4 mainCameraViewMatrix = MakeInVerse(mainCameraMatrix);
	Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(0.45f, static_cast<float>(WinApp::kWindoWidth) / static_cast<float>(WinApp::kWindoHeight), 0.1f, 100.0f);

	Matrix4x4 viewMatrix2D = MakeIdentityMatrix();
	Matrix4x4 projectionMatrix2D = MakeOrthoGraphicsMatrix(0.0f, 0.0f, static_cast<float>(WinApp::kWindoWidth), static_cast<float>(WinApp::kWindoHeight), 0.0f, 100.0f);

	Transform triangleTransform = {};
	triangleTransform.scale = Vector3(1.0f, 1.0f, 1.0f);
	//triangleTransform.translate.x = 100.0f;
	Matrix4x4 triangleWorldMatrix = MakeIdentityMatrix();

	Transform spriteTransform = {};
	spriteTransform.scale = Vector3(0.5f, 0.5f, 0.5f);
	Matrix4x4 spriteWorldMatrix = MakeIdentityMatrix();

	Transform modelTransform = {};
	modelTransform.scale = Vector3(1.0f, 1.0f, 1.0f);
	modelTransform.rotate.y = -1.7f;
	Matrix4x4 modelWorldMatrix = MakeIdentityMatrix();

	Vector4 texColor = materialDataTriangle->Kd;
#pragma endregion

	while (!winApp->PoccesMessage())
	{
		viewport.Width = static_cast<FLOAT>(WinApp::kWindoWidth);
		viewport.Height = static_cast<FLOAT>(WinApp::kWindoHeight);

		scissorRect.right = WinApp::kWindoWidth;
		scissorRect.bottom = WinApp::kWindoHeight;

#pragma region Begin Frame
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		auto& io = ImGui::GetIO();
		io.DisplaySize = ImVec2(static_cast<float>(WinApp::kWindoWidth), static_cast<float>(WinApp::kWindoHeight));
#pragma endregion

#pragma region Imgui Update
		ImGui::Begin("Debug");
		ImGui::DragFloat3("Main Camera Position", &mainCameraTransform.translate.x, 0.1f);
		ImGui::DragFloat4("Tex Color", &texColor.x, 0.001f, 0.0f, 1.0f);
		ImGui::DragFloat3("Model Rot", &modelTransform.rotate.x, 0.01f);
		ImGui::DragFloat3("Light Dir", &directionalLightData->direction.x, 0.01f);
		ImGui::End();

		ImGui::Render();
#pragma endregion

		winApp->Update();

#pragma region GameUpdate
		projectionMatrix = MakePerspectiveFovMatrix(0.45f, static_cast<float>(WinApp::kWindoWidth) / static_cast<float>(WinApp::kWindoHeight), 0.1f, 100.0f);
		projectionMatrix2D = MakeOrthoGraphicsMatrix(0.0f, 0.0f, static_cast<float>(WinApp::kWindoWidth), static_cast<float>(WinApp::kWindoHeight), 0.0f, 100.0f);

		mainCameraMatrix = MakeAffineMatrix(mainCameraTransform.scale, mainCameraTransform.rotate, mainCameraTransform.translate);
		mainCameraViewMatrix = MakeInVerse(mainCameraMatrix);

		materialDataTriangle->Kd = texColor;

		triangleTransform.rotate.y += 0.01f;
		triangleWorldMatrix = MakeAffineMatrix(triangleTransform.scale, triangleTransform.rotate, triangleTransform.translate);
		wvpDataTriangle->wvp = triangleWorldMatrix * mainCameraViewMatrix * projectionMatrix;
		wvpDataTriangle->world = triangleWorldMatrix;

		modelTransform.rotate.y += 0.03f;
		modelWorldMatrix = MakeAffineMatrix(modelTransform.scale, modelTransform.rotate, modelTransform.translate);
		wvpDataModel->wvp = modelWorldMatrix * mainCameraViewMatrix * projectionMatrix;
		wvpDataModel->world = modelWorldMatrix;

		/*spriteWorldMatrix = MakeAffineMatrix(spriteTransform.scale, spriteTransform.rotate, spriteTransform.translate);
		wvpDataSprite->wvp = spriteWorldMatrix * viewMatrix2D * projectionMatrix2D;
		wvpDataSprite->world = MakeIdentityMatrix();*/
#pragma endregion

#pragma region PreDraw
		viewport.Width = static_cast<FLOAT>(WinApp::kWindoWidth);
		viewport.Height = static_cast<FLOAT>(WinApp::kWindoHeight);
		command->GetCommandList()->RSSetViewports(1, &viewport);
		scissorRect.right = WinApp::kWindoWidth;
		scissorRect.bottom = WinApp::kWindoHeight;
		command->GetCommandList()->RSSetScissorRects(1, &scissorRect);

		// Transition Barrier Set
		command->GetCommandList()->ResourceBarrier(1, swapChain->GetBarrier(D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
		backBufferIndex = swapChain->GetBackBufferIndex();

		// 描画先のRTｖをバックバッファのインデックスをもとに設定
		command->GetCommandList()->OMSetRenderTargets(1, &rtvHandles[backBufferIndex], false, nullptr);
		// 画面全体をクリア
		command->GetCommandList()->ClearRenderTargetView(rtvHandles[backBufferIndex], clearColor, 0, nullptr);

		command->GetCommandList()->OMSetRenderTargets(1, &rtvHandles[backBufferIndex], false, &dsvHandle);
		command->GetCommandList()->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

		ID3D12DescriptorHeap* descriptorHeaps[] = { srvDescriptorHeap.Get() };
		command->GetCommandList()->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

		command->GetCommandList()->RSSetViewports(1, &viewport);
		command->GetCommandList()->RSSetScissorRects(1, &scissorRect);
		command->GetCommandList()->SetGraphicsRootSignature(rootSignature->GetRootSignature());
		command->GetCommandList()->SetPipelineState(pipelineState->GetPipelineState());
		command->GetCommandList()->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
#pragma endregion

#pragma region 3D Draw
		command->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferViewTriangle);

		// CBuffer Set
		rootSignature->SetGraphicsCommands(command.get(), monsterBallTex.texSrvHandleGPU);
		
		// いざ描画
		command->GetCommandList()->DrawInstanced(6, 1, 0, 0);

		for (size_t i = 0; i < modelData.objects.size(); ++i)
		{
			command->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferViewModel[i]);

			// CBuffer Set
			transformationMatrix.ptr = wvpDataModel;
			material.ptr = materialDataModel;
			rootSignature->SetGraphicsCommands(command.get(), modelData.objects[i].texData.texSrvHandleGPU);
			// いざ描画
			command->GetCommandList()->DrawInstanced(static_cast<UINT>(modelData.objects[i].vertices.size()), 1, 0, 0);
		}
#pragma endregion

#pragma region 2D Draw
		//command->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferViewSprite);
		//// CBuffer Set
		//command->GetCommandList()->SetGraphicsRootConstantBufferView(pipelineState->GetRootParamIndex("defVertex"), wvpResourceSprite->GetGPUVirtualAddress());
		//command->GetCommandList()->SetGraphicsRootConstantBufferView(pipelineState->GetRootParamIndex("defMtl"), materialResourceSprite->GetGPUVirtualAddress());
		//command->GetCommandList()->SetGraphicsRootDescriptorTable(pipelineState->GetRootParamIndex("defTex"), monsterBallTex.texSrvHandleGPU);
		//command->GetCommandList()->SetGraphicsRootConstantBufferView(pipelineState->GetRootParamIndex("defLight"), directionalLightResource->GetGPUVirtualAddress());
		//// いざ描画
		//command->GetCommandList()->DrawInstanced(6, 1, 0, 0);
#pragma endregion

#pragma region PostDraw
#pragma region ImGui Set
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), command->GetCommandList());
#pragma endregion

#pragma region TransitionBarrier Change To State
		// Windor Drawing Step
		// State Render -> Present
		// Transition Barrier Set
		command->GetCommandList()->ResourceBarrier(1, swapChain->GetBarrier(D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
#pragma endregion
#pragma region GPU Work Transfer & Wait
		// CommandList Close & Kick
		command->Close();
		// GPUとOSに画面の交換を依頼を通知
		swapChain->Present(1, 0);
		// Fence Wait
		fence->IncrementFenceValue();
		command->GetCommandQueue()->Signal(fence->GetFence(), fence->GetFenceValue());
		fence->WaitSignalToGPU();
		// commandList Reset
		command->Reset();
#pragma endregion
	}
#pragma region Finalize
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	/*materialResourceModel->Unmap(0, nullptr);
	wvpResourceModel->Unmap(0, nullptr);
	for (ComPtr<ID3D12Resource> resource : vertexResourceModel)
	{
		resource->Unmap(0, nullptr);
	}*/

	/*materialResourceSprite->Unmap(0, nullptr);
	wvpResourceSprite->Unmap(0, nullptr);
	vertexResourceSprite->Unmap(0, nullptr);*/

	texManager->Finalize();

	winApp->Finalize();
	// COM終了
	CoUninitialize();
#pragma endregion
	return 0;
}

/// <summary>
/// ミップマップ付きデータの取得
/// </summary>
/// <param name="filePath"></param>
/// <returns></returns>
DirectX::ScratchImage LoadTexture(const std::string& filePath)
{
	// TextureFileえおプログラム用に読み込む
	DirectX::ScratchImage image{};
	std::wstring filePathW = StringUtility::ConvertToWString(filePath);
	HRESULT hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
	assert(SUCCEEDED(hr));
	Logger::Log("Texture Load\n");

	DirectX::ScratchImage mipImage{};
	hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImage);
	Logger::Log("MipMap Create\n");
	return mipImage;
}

/// <summary>
/// テクスチャリソースの作成
/// </summary>
/// <param name="device">作成してくれるデバイス</param>
/// <param name="metaData">作成元データ</param>
/// <returns></returns>
ComPtr<ID3D12Resource> CreateTextureResource(const ComPtr<ID3D12Device>& device, const DirectX::TexMetadata& metaData)
{
	// metaDataからResourceの設定を取得
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = static_cast<UINT>(metaData.width);
	resourceDesc.Height = static_cast<UINT>(metaData.height);
	resourceDesc.MipLevels = static_cast<UINT16>(metaData.mipLevels);
	resourceDesc.DepthOrArraySize = static_cast<UINT16>(metaData.arraySize);
	resourceDesc.Format = metaData.format;
	resourceDesc.SampleDesc.Count = 1; // サンプリングカウント1固定
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION(metaData.dimension);

	// Heap設定
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	//heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;	// writeBackポリシーでcpuアクセス許可
	//heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;				// プロセッサの近くに配置

	// Resource生成
	ComPtr<ID3D12Resource> resource = nullptr;
	HRESULT hr = device->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_COPY_DEST, // データ転送設定
		nullptr,
		IID_PPV_ARGS(&resource)
	);
	assert(SUCCEEDED(hr));
	Logger::Log("TextureResource Created\n");
	return resource;
}

/// <summary>
/// 中間リソースの作成とアップロード
/// </summary>
/// <param name="texture">中間リソースを作成するリソース</param>
/// <param name="mipImage">元データ</param>
/// <param name="device">作成してくれるデバイス</param>
/// <param name="commandList">アップロードコマンド積込みと実行用</param>
/// <returns>中間リソース転送完了まで破棄しないこと</returns>
[[nodiscard]]
ComPtr<ID3D12Resource> UploadTextureData(const ComPtr<ID3D12Resource>& texture, const DirectX::ScratchImage& mipImage, const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12GraphicsCommandList>& commandList)
{
	// 中間リソースの作成までを別関数にわかるべきか？
	std::vector<D3D12_SUBRESOURCE_DATA> subresources;
	DirectX::PrepareUpload(device.Get(), mipImage.GetImages(), mipImage.GetImageCount(), mipImage.GetMetadata(), subresources);
	uint64_t intermediateSize = GetRequiredIntermediateSize(texture.Get(), 0, static_cast<UINT>(subresources.size()));
	ComPtr<ID3D12Resource> intermediateResource = Dx12ObjFuncs::CreataeBufferResource(device, intermediateSize);

	// どうやったらこの関数の使用をやめれる？
	UpdateSubresources(commandList.Get(), texture.Get(), intermediateResource.Get(), 0, 0, static_cast<UINT>(subresources.size()), subresources.data());

	// プロシージャかなんかで裏で待機させたいよね
	// 転送後、コピーからリードへ変更
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = texture.Get();
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;
	commandList->ResourceBarrier(1, &barrier);
	Logger::Log("MipMap Upload To Texture\n");
	return intermediateResource;
}

/// <summary>
/// 深度バッファ用リソースの作成
/// </summary>
/// <param name="device"></param>
/// <param name="width"></param>
/// <param name="height"></param>
/// <returns></returns>
ComPtr<ID3D12Resource> CreateDepthStencilTextureResource(const ComPtr<ID3D12Device>& device, int32_t width, int32_t height)
{
	// metaDataからResourceの設定を取得
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = width;
	resourceDesc.Height = height;
	resourceDesc.MipLevels = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	resourceDesc.SampleDesc.Count = 1; // サンプリングカウント1固定
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	// Heap設定
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

	// 深度値のクリア設定
	CD3DX12_CLEAR_VALUE depthClearValue{};
	depthClearValue.DepthStencil.Depth = 1.0f;				// 最大値でクリア
	depthClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;	// Resourceとあわせる

	// Resource生成
	ComPtr<ID3D12Resource> resource = nullptr;
	HRESULT hr = device->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE, // データ転送設定
		&depthClearValue,
		IID_PPV_ARGS(&resource)
	);
	assert(SUCCEEDED(hr));
	Logger::Log("TextureResource Created\n");
	return resource;
}

D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(const ComPtr<ID3D12DescriptorHeap>& descriptorHeap, uint32_t descriptorSize, uint32_t index)
{
	D3D12_CPU_DESCRIPTOR_HANDLE result = descriptorHeap->GetCPUDescriptorHandleForHeapStart();;
	result.ptr += static_cast<SIZE_T>(descriptorSize * index);
	return result;
}

D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(const ComPtr<ID3D12DescriptorHeap>& descriptorHeap, uint32_t descriptorSize, uint32_t index)
{
	D3D12_GPU_DESCRIPTOR_HANDLE result = descriptorHeap->GetGPUDescriptorHandleForHeapStart();;
	result.ptr += static_cast<UINT64>(descriptorSize * index);
	return result;
}

/// <summary>
/// objファイルの読み込み
/// </summary>
/// <param name="filePath">.objファイルのパス</param>
/// <returns>モデルの頂点情報</returns>
ModelData LoadObjFile(const std::string& directoryPath, const std::string& filePath)
{
	ModelData result;
	std::unique_ptr<ObjectData> obj = std::make_unique<ObjectData>();
	std::vector<Vector4> positions;
	std::vector<Vector3> normals;
	std::vector<Vector2> uvs;
	std::string useMtlFileName;

	std::string line;
	std::ifstream file(directoryPath + '/' + filePath);
	assert(file.is_open());

	while (std::getline(file, line))
	{
		std::string identifier;
		std::istringstream s(line);
		s >> identifier; // 先頭識別子読み込み

		if (identifier == "mtllib")
		{
			s >> useMtlFileName;
		}
		// オブジェクト名
		else if (identifier == "o" || identifier == "g") {
			// 名前が空じゃない = 内容がある
			// 返却用構造体にpuahして内容破棄
			if (!obj->name.empty()) {
				result.objects.push_back(*obj);
				obj.release();
				obj = std::make_unique<ObjectData>();
			}
			s >> obj->name;
		}
		// 頂点
		else if (identifier == "v")
		{
			Vector4 position;
			s >> position.x >> position.y >> position.z;
			position.x *= -1.0f;
			position.w = 1.0f;
			positions.push_back(position);
		}
		// uv
		else if (identifier == "vt")
		{
			Vector2 uv;
			s >> uv.x >> uv.y;
			uv.y = 1.0f - uv.y;
			uvs.push_back(uv);
		}
		// 法線
		else if (identifier == "vn")
		{
			Vector3 normal;
			s >> normal.x >> normal.y >> normal.z;
			normal.x *= -1.0f;
			normals.push_back(normal);
		}
		// 使用マテリアル名
		else if (identifier == "usemtl") {
			std::string useMatreialName;
			s >> useMatreialName;
			obj->material = LoadMtlFile(directoryPath + '/' + useMtlFileName, useMatreialName);
		}
		// index
		else if (identifier == "f")
		{
			// とりあえず今は三角のみ
			const uint32_t triangleVertex = 3;
			VertexData triangle[triangleVertex];
			for (int32_t faceVertex = 0; faceVertex < triangleVertex; ++faceVertex)
			{
				std::string vertexDefinition;
				s >> vertexDefinition;
				// [頂点 / UV / 法線]で格納されているため分解してindexを取得
				std::istringstream v(vertexDefinition);
				uint32_t elementIndecies[3]{};
				for (int32_t element = 0; element < 3; ++element)
				{
					std::string index;
					std::getline(v, index, '/'); // スラッシュ区切りで読み込み
					elementIndecies[element] = std::stoi(index);
				}
				uint32_t positionIndex = elementIndecies[0] - 1;
				uint32_t uvIndex = elementIndecies[1] - 1;
				uint32_t normalIndex = elementIndecies[2] - 1;
				Vector4 position = positions[positionIndex];
				Vector2 uv = uvs[uvIndex];
				Vector3 normal = normals[normalIndex];
				triangle[faceVertex] = { position, uv, normal };
			}

			for (int i = triangleVertex - 1; i >= 0; --i)
			{
				obj->vertices.push_back(triangle[i]);
			}
		}
	}

	result.objects.push_back(*obj);
	return result;
}

MaterialData LoadMtlFile(const std::string& fileName, const std::string& useMaterialName)
{
	MaterialData result{};

	std::string line;
	std::ifstream file(fileName);

	std::unique_ptr<std::string> materialName = std::make_unique<std::string>();

	while (std::getline(file, line))
	{
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;
		if (identifier == "newmtl")
		{
			s >> *materialName;
		}
		if (useMaterialName == *materialName)
		{
			if (identifier == "Ns")
			{

			}
			else if (identifier == "Ns")
			{

			}
			else if (identifier == "Ka")
			{
				s >> result.Ka.x >> result.Ka.y >> result.Ka.z;
				result.Ka.w = 1.0f;
			}
			else if (identifier == "Kd")
			{
				s >> result.Kd.x >> result.Kd.y >> result.Kd.z;
				result.Kd.w = 1.0f;
			}
			else if (identifier == "Ks")
			{
				s >> result.Ks.x >> result.Ks.y >> result.Ks.z;
				result.Ks.w = 1.0f;
			}
			else if (identifier == "Ke")
			{

			}
			else if (identifier == "Ni")
			{

			}
			else if (identifier == "ilumi")
			{

			}
			else if (identifier == "map_Kd")
			{
				s >> result.textureFileName;
			}
		}
	}
	return result;
}
