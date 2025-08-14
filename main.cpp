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
#include "Engine//DirectX12Objects/DxFence.h"
#include "Engine/DirectX12Objects/DxShader.h"
#include "Engine/DxRenderContext.h"
#include "Engine//TextureManager.h"

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
	std::string textureFilePath;
};

struct ObjectData
{
	std::string name;
	std::vector<VertexData> vertices;
	MaterialData material{};
};

struct ModelData
{
	std::vector<ObjectData> objects;
};

//DirectX::ScratchImage LoadTexture(const std::string& filePath);
//ComPtr<ID3D12Resource> CreateTextureResource(const ComPtr<ID3D12Device>& device, const DirectX::TexMetadata& metaData);
//ComPtr<ID3D12Resource> UploadTextureData(const ComPtr<ID3D12Resource>& texture, const DirectX::ScratchImage& mipImage, const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12GraphicsCommandList>& commandList);

ModelData LoadObjFile(const std::string& directoryPath, const std::string& filePath);
MaterialData LoadMtlFile(const std::string& fileName, const std::string& useMaterialName);
#pragma endregion

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int)
{
	// Texture読み込みのためCOMを初期化


	unique_ptr<WinApp> winApp = make_unique<WinApp>();
	winApp->Initialize();

	unique_ptr<DxRenderContext> renderContext = make_unique<DxRenderContext>();
	renderContext->Initialize(*winApp.get());

	TextureManager::GetInstace().Initialize(renderContext.get());

	unique_ptr<DxShader> shaderManager = make_unique<DxShader>();
	shaderManager->Initialize();

#pragma region Shader Compile
	const std::string shaderDirectoryPath = "Resources/Shaders/";
	ComPtr<IDxcBlob> vertexShaderBlob = shaderManager->CompileShader(shaderDirectoryPath + "Basic3DVS.hlsl", L"vs_6_0");
	assert(vertexShaderBlob != nullptr);
	ComPtr<IDxcBlob> pixelShaderBlob = shaderManager->CompileShader(shaderDirectoryPath + "Basic3DPS.hlsl", L"ps_6_0");
	assert(pixelShaderBlob != nullptr);
#pragma endregion

#pragma region RootParameter Create
	D3D12_ROOT_PARAMETER rootParameters[4] = {};
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[0].Descriptor.ShaderRegister = 0;

	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameters[1].Descriptor.ShaderRegister = 0;

	// Texture用
	D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
	descriptorRange[0].BaseShaderRegister = 0;
	descriptorRange[0].NumDescriptors = 1;
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[2].DescriptorTable.pDescriptorRanges = descriptorRange;
	rootParameters[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);

	rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[3].Descriptor.ShaderRegister = 1;
#pragma endregion
#pragma region Smapler Settings
	D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;			// バイナリフィルタ
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;		// 0~1リピート
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;		// 
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;		// 
	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;		// 比較しない
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;						// 最大まで使用
	staticSamplers[0].ShaderRegister = 0;
	staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
#pragma endregion
#pragma region RootSignature Create
	D3D12_ROOT_SIGNATURE_DESC descriptorRootSignature{};
	descriptorRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	descriptorRootSignature.pParameters = rootParameters;
	descriptorRootSignature.NumParameters = _countof(rootParameters);
	descriptorRootSignature.pStaticSamplers = staticSamplers;
	descriptorRootSignature.NumStaticSamplers = _countof(staticSamplers);
	// シリアライズしてバイナリ化
	ComPtr<ID3DBlob> signatureBlob = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&descriptorRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED(hr))
	{
		Logger::Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);
	}
	// バイナリを基に生成
	ComPtr<ID3D12RootSignature> rootSignature = nullptr;
	hr = renderContext->GetDxDevice()->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
	assert(SUCCEEDED(hr));
	Logger::Log("Created RootSignature\n");
#pragma endregion
#pragma region InputLayout Settings
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[3] = {};
	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs[1].SemanticName = "TEXCOORD";
	inputElementDescs[1].SemanticIndex = 0;
	inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs[2].SemanticName = "NORMAL";
	inputElementDescs[2].SemanticIndex = 0;
	inputElementDescs[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDescs[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = _countof(inputElementDescs);
#pragma endregion
#pragma region BlendState Settings
	D3D12_BLEND_DESC blendDesc{};
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
#pragma endregion
#pragma region RasterizerState Settings
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	// カリングモード設定
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	// 塗りつぶし
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
#pragma endregion
#pragma region DepthStencilState Settings
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	depthStencilDesc.DepthEnable = true;							// 深度機能有効化
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;	// 書き込みする
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;	// Depthの値が小さい方が描画される
#pragma endregion
#pragma region PSO Create
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDec{};
	graphicsPipelineStateDec.pRootSignature = rootSignature.Get();
	graphicsPipelineStateDec.InputLayout = inputLayoutDesc;
	graphicsPipelineStateDec.VS = { vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize() };
	graphicsPipelineStateDec.PS = { pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize() };
	graphicsPipelineStateDec.BlendState = blendDesc;
	graphicsPipelineStateDec.RasterizerState = rasterizerDesc;
	// 書き込むRTVの情報
	graphicsPipelineStateDec.NumRenderTargets = 1;
	graphicsPipelineStateDec.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	// 利用するトポロジタイプ
	graphicsPipelineStateDec.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	// 色の打ち込み方設定
	graphicsPipelineStateDec.SampleDesc.Count = 1;
	graphicsPipelineStateDec.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	// 深度情報設定
	graphicsPipelineStateDec.DepthStencilState = depthStencilDesc;
	graphicsPipelineStateDec.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	// 生成
	ComPtr<ID3D12PipelineState> graphicsPipelineState = nullptr;
	hr = renderContext->GetDxDevice()->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDec, IID_PPV_ARGS(&graphicsPipelineState));
	assert(SUCCEEDED(hr));
	Logger::Log("Created PSO\n");
#pragma endregion

#pragma endregion

#pragma region Model Load
	//ModelData modelData = LoadObjFile("Resources/Models/Syunnya_Tamura/Shield", "cubes.obj");
	ModelData modelData = LoadObjFile("Resources/Models", "multiMesh.obj");
	for (ObjectData& obj : modelData.objects)
	{
		if (obj.material.textureFilePath.empty())
		{
			obj.material.textureFilePath = "uvChecker.png";
		}
	}
#pragma endregion

	TextureDataCPU textureDataCPU = TextureManager::GetInstace().Load("Resources/Models/", modelData.objects[0].material.textureFilePath);
	TextureDataCPU textureDataCPU2 = TextureManager::GetInstace().Load("uvChecker.png");

#pragma region TextureResource Create

	// Textureを読み込んで転送
	//DirectX::ScratchImage mipImages = LoadTexture("Resources/Models/Syunnya_Tamura/shield/shield_Allin_BaseColor.png");
	//DirectX::ScratchImage mipImages = LoadTexture("Resources/Textures/" + modelData.objects[0].material.textureFilePath);
	//const DirectX::TexMetadata& metaData = mipImages.GetMetadata();
	//ComPtr<ID3D12Resource> textureResource = renderContext->CreateTextureResource(metaData);
	//ComPtr<ID3D12Resource> intermediateResource = renderContext->UploadTextureData(textureResource, mipImages);

	// Textureを読み込んで転送
	/*DirectX::ScratchImage mipImages2 = LoadTexture("Resources/Textures/uvChecker.png");
	const DirectX::TexMetadata& metaData2 = mipImages2.GetMetadata();
	ComPtr<ID3D12Resource> textureResource2 = CreateTextureResource(renderContext->GetDxDevice()->GetDevice(), metaData2);
	ComPtr<ID3D12Resource> intermediateResource2 = UploadTextureData(textureResource2, mipImages2, renderContext->GetDxDevice()->GetDevice(), renderContext->GetCommand()->GetCommandList());

	renderContext->WaitForGPU();*/

#pragma endregion

#pragma region textureSRV Create
	/*D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = metaData.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = static_cast<UINT>(metaData.mipLevels);

	D3D12_CPU_DESCRIPTOR_HANDLE texSrvHandleCPU = renderContext->GetSRVDescriptorCPUHandle(1);
	D3D12_GPU_DESCRIPTOR_HANDLE texSrvHandleGPU = renderContext->GetSRVDescriptorGPUHandle(1);

	renderContext->GetDxDevice()->GetDevice()->CreateShaderResourceView(textureResource.Get(), &srvDesc, texSrvHandleCPU);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc2{};
	srvDesc2.Format = metaData2.format;
	srvDesc2.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc2.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc2.Texture2D.MipLevels = static_cast<UINT>(metaData2.mipLevels);

	D3D12_CPU_DESCRIPTOR_HANDLE texSrvHandleCPU2 = renderContext->GetSRVDescriptorCPUHandle(2);
	D3D12_GPU_DESCRIPTOR_HANDLE texSrvHandleGPU2 = renderContext->GetSRVDescriptorGPUHandle(2);

	renderContext->GetDxDevice()->GetDevice()->CreateShaderResourceView(textureResource2.Get(), &srvDesc2, texSrvHandleCPU2);*/
#pragma endregion

#pragma region Resources Create
	ComPtr<ID3D12Resource> vertexResourceTriangle =renderContext->CreataeBufferResource(sizeof(VertexData) * 6);
	ComPtr<ID3D12Resource> wvpResourceTriangle = renderContext->CreataeBufferResource(sizeof(TransformationMatrix));
	ComPtr<ID3D12Resource> materialResourceTriangle = renderContext->CreataeBufferResource(sizeof(MaterialData));

	ComPtr<ID3D12Resource> vertexResourceSprite = renderContext->CreataeBufferResource(sizeof(VertexData) * 6);
	ComPtr<ID3D12Resource> wvpResourceSprite = renderContext->CreataeBufferResource(sizeof(TransformationMatrix));
	ComPtr<ID3D12Resource> materialResourceSprite = renderContext->CreataeBufferResource(sizeof(MaterialData));

	//ComPtr<ID3D12Resource> vertexResourceModel = DirectX12ObjectsFunction::CreataeBufferResource(device->GetDevice(), sizeof(VertexData) * modelData.objects[1].vertices.size());
	std::vector<ComPtr<ID3D12Resource>> vertexResourceModel;
	vertexResourceModel.reserve(modelData.objects.size());
	for (ObjectData obj : modelData.objects)
	{
		vertexResourceModel.push_back(renderContext->CreataeBufferResource(sizeof(VertexData) * obj.vertices.size()));
	}
	ComPtr<ID3D12Resource> wvpResourceModel = renderContext->CreataeBufferResource(sizeof(TransformationMatrix));
	ComPtr<ID3D12Resource> materialResourceModel = renderContext->CreataeBufferResource(sizeof(MaterialData));

	ComPtr<ID3D12Resource> directionalLightResource = renderContext->CreataeBufferResource(sizeof(DirectionalLight));
#pragma endregion

#pragma region Resources Writing

#pragma region Trinangle
	VertexData* vertexDataTriangle = nullptr;
	// 書き込み先アドレス取得
	vertexResourceTriangle->Map(0, nullptr, reinterpret_cast<void**>(&vertexDataTriangle));
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

	TransformationMatrix* wvpDataTriangle = nullptr;
	wvpResourceTriangle->Map(0, nullptr, reinterpret_cast<void**>(&wvpDataTriangle));
	wvpDataTriangle->wvp = MakeIdentityMatrix();

	MaterialData* materialDataTriangle = nullptr;
	materialResourceTriangle->Map(0, nullptr, reinterpret_cast<void**>(&materialDataTriangle));
	materialDataTriangle->Kd = Vector4(0.0f, 0.5f, 0.5f, 1.0f);
#pragma endregion

#pragma region Sprite
	VertexData* vertexDataSprite = nullptr;
	// 書き込み先アドレス取得
	vertexResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&vertexDataSprite));
	vertexDataSprite[0].position = { 0.0f, static_cast<float>(textureDataCPU2.metaData.height), 0.0f, 1.0f };								// left bottom
	vertexDataSprite[0].uv = { 0.0f, 1.0f };
	vertexDataSprite[1].position = { 0.0f, 0.0f, 0.0f, 1.0f };																// left top
	vertexDataSprite[1].uv = { 0.0f, 0.0f };
	vertexDataSprite[2].position = { static_cast<float>(textureDataCPU2.metaData.width), static_cast<float>(textureDataCPU2.metaData.height), 0.0f, 1.0f };	// right bottom
	vertexDataSprite[2].uv = { 1.0f, 1.0f };

	vertexDataSprite[3].position = { 0.0f, 0.0f, 0.0f, 1.0f };																// left top
	vertexDataSprite[3].uv = { 0.0f, 0.0f };
	vertexDataSprite[4].position = { static_cast<float>(textureDataCPU2.metaData.width), 0.0f, 0.0f, 1.0f };								// right top
	vertexDataSprite[4].uv = { 1.0f, 0.0f };
	vertexDataSprite[5].position = { static_cast<float>(textureDataCPU2.metaData.width), static_cast<float>(textureDataCPU2.metaData.height), 0.0f, 1.0f };	// right bottom	2
	vertexDataSprite[5].uv = { 1.0f, 1.0f };

	TransformationMatrix* wvpDataSprite = nullptr;
	wvpResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&wvpDataSprite));
	wvpDataSprite->wvp = MakeIdentityMatrix();

	MaterialData* materialDataSprite = nullptr;
	materialResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&materialDataSprite));
	materialDataSprite->Kd = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	materialDataSprite->enableLighting = false;
#pragma endregion

#pragma region Model
	std::vector<VertexData*> vertexDataModel;
	vertexDataModel.resize(modelData.objects.size());
	for (size_t i = 0; i < modelData.objects.size(); ++i)
	{
		vertexResourceModel[i]->Map(0, nullptr, reinterpret_cast<void**>(&vertexDataModel[i]));
		// 頂点データコピー
		if (vertexDataModel[i] != nullptr)
		{
			std::memcpy(vertexDataModel[i], modelData.objects[i].vertices.data(), sizeof(VertexData) * modelData.objects[i].vertices.size());
		}
	}

	TransformationMatrix* wvpDataModel = nullptr;
	wvpResourceModel->Map(0, nullptr, reinterpret_cast<void**>(&wvpDataModel));
	wvpDataModel->wvp = MakeIdentityMatrix();

	MaterialData* materialDataModel = nullptr;
	materialResourceModel->Map(0, nullptr, reinterpret_cast<void**>(&materialDataModel));
	*materialDataModel = modelData.objects[0].material;
	materialDataModel->Kd = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	//materialDataModel->enableLighting = false;
#pragma endregion

	DirectionalLight* directionalLightData = nullptr;
	directionalLightResource->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData));
	directionalLightData->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	directionalLightData->direction = Vector3(0.0f, -1.0f, 0.0f);
	directionalLightData->intensity = 1.0f;

#pragma endregion

#pragma region VertexBufferView Create
	D3D12_VERTEX_BUFFER_VIEW vertexBufferViewTriangle{};
	vertexBufferViewTriangle.BufferLocation = vertexResourceTriangle->GetGPUVirtualAddress();
	vertexBufferViewTriangle.SizeInBytes = sizeof(VertexData) * 6;
	vertexBufferViewTriangle.StrideInBytes = sizeof(VertexData);

	D3D12_VERTEX_BUFFER_VIEW vertexBufferViewSprite{};
	vertexBufferViewSprite.BufferLocation = vertexResourceSprite->GetGPUVirtualAddress();
	vertexBufferViewSprite.SizeInBytes = sizeof(VertexData) * 6;
	vertexBufferViewSprite.StrideInBytes = sizeof(VertexData);

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


#pragma region ImGui Initialize
	// こういうもん
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(winApp->GetHWND());
	ImGui_ImplDX12_Init(
		renderContext->GetDxDevice()->GetDevice(),
		renderContext->GetSwapChain()->kBufferCount,
		renderContext->GetRTVDesc().Format, renderContext->GetSRVDescriptorHeap(),
		renderContext->GetSRVDescriptorCPUHandle(0),
		renderContext->GetSRVDescriptorGPUHandle(0)
	);
#pragma endregion

#pragma region 変数宣言
	Transform mainCameraTransform = {};
	mainCameraTransform.scale = Vector3(1.0f, 1.0f, 1.0f);
	mainCameraTransform.translate.z = -200.0f;
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

#pragma region Begin Frame
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

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

		//modelTransform.rotate.y += 0.03f;
		modelWorldMatrix = MakeAffineMatrix(modelTransform.scale, modelTransform.rotate, modelTransform.translate);
		wvpDataModel->wvp = modelWorldMatrix * mainCameraViewMatrix * projectionMatrix;
		wvpDataModel->world = modelWorldMatrix;

		spriteWorldMatrix = MakeAffineMatrix(spriteTransform.scale, spriteTransform.rotate, spriteTransform.translate);
		wvpDataSprite->wvp = spriteWorldMatrix * viewMatrix2D * projectionMatrix2D;
		wvpDataSprite->world = MakeIdentityMatrix();
#pragma endregion

		renderContext->BeginRendering();
		renderContext->GetCommand()->GetCommandList()->SetGraphicsRootSignature(rootSignature.Get());
		renderContext->GetCommand()->GetCommandList()->SetPipelineState(graphicsPipelineState.Get());

#pragma region 3D Draw
		renderContext->GetCommand()->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferViewTriangle);
		// CBuffer Set
		renderContext->GetCommand()->GetCommandList()->SetGraphicsRootConstantBufferView(1, wvpResourceTriangle->GetGPUVirtualAddress());
		renderContext->GetCommand()->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResourceTriangle->GetGPUVirtualAddress());
		renderContext->GetCommand()->GetCommandList()->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstace().GetSRVDescriptorGPUHandle(textureDataCPU.fileName));
		renderContext->GetCommand()->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());
		// いざ描画
		renderContext->GetCommand()->GetCommandList()->DrawInstanced(6, 1, 0, 0);

		for (size_t i = 0; i < modelData.objects.size(); ++i)
		{
			renderContext->GetCommand()->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferViewModel[i]);

			// CBuffer Set
			renderContext->GetCommand()->GetCommandList()->SetGraphicsRootConstantBufferView(1, wvpResourceModel->GetGPUVirtualAddress());
			renderContext->GetCommand()->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResourceModel->GetGPUVirtualAddress());
			renderContext->GetCommand()->GetCommandList()->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstace().GetSRVDescriptorGPUHandle(textureDataCPU.fileName));
			renderContext->GetCommand()->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());
			// いざ描画
			renderContext->GetCommand()->GetCommandList()->DrawInstanced(static_cast<UINT>(modelData.objects[i].vertices.size()), 1, 0, 0);
		}
#pragma endregion

#pragma region 2D Draw
		renderContext->GetCommand()->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferViewSprite);
		// CBuffer Set
		renderContext->GetCommand()->GetCommandList()->SetGraphicsRootConstantBufferView(1, wvpResourceSprite->GetGPUVirtualAddress());
		renderContext->GetCommand()->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResourceSprite->GetGPUVirtualAddress());
		renderContext->GetCommand()->GetCommandList()->SetGraphicsRootDescriptorTable(2, TextureManager::GetInstace().GetSRVDescriptorGPUHandle(textureDataCPU2.fileName));
		renderContext->GetCommand()->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());
		// いざ描画
		renderContext->GetCommand()->GetCommandList()->DrawInstanced(6, 1, 0, 0);
#pragma endregion

#pragma region PostDraw
#pragma region ImGui Set
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), renderContext->GetCommand()->GetCommandList());
#pragma endregion

		renderContext->EndRendering();
	}
#pragma region Finalize
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	materialResourceModel->Unmap(0, nullptr);
	wvpResourceModel->Unmap(0, nullptr);
	for (ComPtr<ID3D12Resource> resource : vertexResourceModel)
	{
		resource->Unmap(0, nullptr);
	}

	materialResourceSprite->Unmap(0, nullptr);
	wvpResourceSprite->Unmap(0, nullptr);
	vertexResourceSprite->Unmap(0, nullptr);

	materialResourceTriangle->Unmap(0, nullptr);
	wvpResourceTriangle->Unmap(0, nullptr);
	vertexResourceTriangle->Unmap(0, nullptr);

	TextureManager::GetInstace().Finalize();

	winApp->Finalize();

#pragma endregion

	return 0;
}

/// <summary>
/// ミップマップ付きデータの取得
/// </summary>
/// <param name="filePath"></param>
/// <returns></returns>
//DirectX::ScratchImage LoadTexture(const std::string& filePath)
//{
//	// TextureFileえおプログラム用に読み込む
//	DirectX::ScratchImage image{};
//	std::wstring filePathW = StringUtility::ConvertToWString(filePath);
//	HRESULT hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
//	assert(SUCCEEDED(hr));
//	Logger::Log("Texture Load\n");
//
//	DirectX::ScratchImage mipImage{};
//	hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImage);
//	Logger::Log("MipMap Create\n");
//	return mipImage;
//}

/// <summary>
/// テクスチャリソースの作成
/// </summary>
/// <param name="device">作成してくれるデバイス</param>
/// <param name="metaData">作成元データ</param>
/// <returns></returns>
//ComPtr<ID3D12Resource> CreateTextureResource(const ComPtr<ID3D12Device>& device, const DirectX::TexMetadata& metaData)
//{
//	// metaDataからResourceの設定を取得
//	D3D12_RESOURCE_DESC resourceDesc{};
//	resourceDesc.Width = static_cast<UINT>(metaData.width);
//	resourceDesc.Height = static_cast<UINT>(metaData.height);
//	resourceDesc.MipLevels = static_cast<UINT16>(metaData.mipLevels);
//	resourceDesc.DepthOrArraySize = static_cast<UINT16>(metaData.arraySize);
//	resourceDesc.Format = metaData.format;
//	resourceDesc.SampleDesc.Count = 1; // サンプリングカウント1固定
//	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION(metaData.dimension);
//
//	// Heap設定
//	D3D12_HEAP_PROPERTIES heapProperties{};
//	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
//	//heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;	// writeBackポリシーでcpuアクセス許可
//	//heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;				// プロセッサの近くに配置
//
//	// Resource生成
//	ComPtr<ID3D12Resource> resource = nullptr;
//	HRESULT hr = device->CreateCommittedResource(
//		&heapProperties,
//		D3D12_HEAP_FLAG_NONE,
//		&resourceDesc,
//		D3D12_RESOURCE_STATE_COPY_DEST, // データ転送設定
//		nullptr,
//		IID_PPV_ARGS(&resource)
//	);
//	assert(SUCCEEDED(hr));
//	Logger::Log("TextureResource Created\n");
//	return resource;
//}

/// <summary>
/// 中間リソースの作成とアップロード
/// </summary>
/// <param name="texture">中間リソースを作成するリソース</param>
/// <param name="mipImage">元データ</param>
/// <param name="device">作成してくれるデバイス</param>
/// <param name="commandList">アップロードコマンド積込みと実行用</param>
/// <returns>中間リソース転送完了まで破棄しないこと</returns>
//[[nodiscard]]
//ComPtr<ID3D12Resource> UploadTextureData(const ComPtr<ID3D12Resource>& texture, const DirectX::ScratchImage& mipImage, const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12GraphicsCommandList>& commandList)
//{
//	// 中間リソースの作成までを別関数にわかるべきか？
//	std::vector<D3D12_SUBRESOURCE_DATA> subresources;
//	DirectX::PrepareUpload(device.Get(), mipImage.GetImages(), mipImage.GetImageCount(), mipImage.GetMetadata(), subresources);
//	uint64_t intermediateSize = GetRequiredIntermediateSize(texture.Get(), 0, static_cast<UINT>(subresources.size()));
//	ComPtr<ID3D12Resource> intermediateResource = DirectX12ObjectsFunction::CreataeBufferResource(device, intermediateSize);
//
//	// どうやったらこの関数の使用をやめれる？
//	UpdateSubresources(commandList.Get(), texture.Get(), intermediateResource.Get(), 0, 0, static_cast<UINT>(subresources.size()), subresources.data());
//
//	// プロシージャかなんかで裏で待機させたいよね
//	// 転送後、コピーからリードへ変更
//	D3D12_RESOURCE_BARRIER barrier{};
//	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
//	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
//	barrier.Transition.pResource = texture.Get();
//	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
//	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
//	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;
//	commandList->ResourceBarrier(1, &barrier);
//	Logger::Log("MipMap Upload To Texture\n");
//	return intermediateResource;
//}

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
		else if (identifier == "o") {
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
				s >> result.textureFilePath;
			}
		}
	}
	return result;
}
