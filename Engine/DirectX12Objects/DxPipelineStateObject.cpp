#include "DxPipelineStateObject.h"

#include <cassert>

#include "DxDevice.h"
#include "DxRootSignature.h"
#include "../Logger.h"

//void DxPipelineStateObject::DefaultInitialize(DxDevice *device)
//{
//
//	std::unique_ptr<DxShaderManager> shaderManager = std::make_unique<DxShaderManager>();
//	shaderManager->Initialize();
//	ComPtr<IDxcBlob> vertexShaderBlob = shaderManager->CompileShader("Basic3DVS.hlsl", L"vs_6_0");
//	ComPtr<IDxcBlob> pixelShaderBlob = shaderManager->CompileShader("Basic3DPS.hlsl", L"ps_6_0");
//
//	rootSignature_ = std::make_unique<DxRootSignature>();
//	//rootSignature_->DefaultSettings();
//	rootSignature_->Create(device);
//#pragma region InputLayout Settings
//	D3D12_INPUT_ELEMENT_DESC inputElementDescs[3] = {};
//	inputElementDescs[0].SemanticName = "POSITION";
//	inputElementDescs[0].SemanticIndex = 0;
//	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
//	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
//	inputElementDescs[1].SemanticName = "TEXCOORD";
//	inputElementDescs[1].SemanticIndex = 0;
//	inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
//	inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
//	inputElementDescs[2].SemanticName = "NORMAL";
//	inputElementDescs[2].SemanticIndex = 0;
//	inputElementDescs[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
//	inputElementDescs[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
//
//	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
//	inputLayoutDesc.pInputElementDescs = inputElementDescs;
//	inputLayoutDesc.NumElements = _countof(inputElementDescs);
//#pragma endregion
//#pragma region BlendState Settings
//	D3D12_BLEND_DESC blendDesc{};
//	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
//#pragma endregion
//#pragma region RasterizerState Settings
//	D3D12_RASTERIZER_DESC rasterizerDesc{};
//	// カリングモード設定
//	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
//	// 塗りつぶし
//	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
//#pragma endregion
//#pragma region DepthStencilState Settings
//	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
//	depthStencilDesc.DepthEnable = true;							// 深度機能有効化
//	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;	// 書き込みする
//	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;	// Depthの値が小さい方が描画される
//#pragma endregion
//#pragma region PSO Create
//	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDec{};
//	graphicsPipelineStateDec.pRootSignature = rootSignature_->GetRootSignature();
//	graphicsPipelineStateDec.InputLayout = inputLayoutDesc;
//	graphicsPipelineStateDec.VS = { vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize() };
//	graphicsPipelineStateDec.PS = { pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize() };
//	graphicsPipelineStateDec.BlendState = blendDesc;
//	graphicsPipelineStateDec.RasterizerState = rasterizerDesc;
//	// 書き込むRTVの情報
//	graphicsPipelineStateDec.NumRenderTargets = 1;
//	graphicsPipelineStateDec.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
//	// 利用するトポロジタイプ
//	graphicsPipelineStateDec.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
//	// 色の打ち込み方設定
//	graphicsPipelineStateDec.SampleDesc.Count = 1;
//	graphicsPipelineStateDec.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
//	// 深度情報設定
//	graphicsPipelineStateDec.DepthStencilState = depthStencilDesc;
//	graphicsPipelineStateDec.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
//	// 生成
//	HRESULT hr = device->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDec, IID_PPV_ARGS(&graphicsPipelineState));
//	assert(SUCCEEDED(hr));
//	Logger::Log("Created PSO\n");
//#pragma endregion
//}

//ID3D12RootSignature* DxPipelineStateObject::GetRootSignature()
//{
//	return rootSignature_->GetRootSignature();
//}

void DxPipelineStateObject::InitializeAndCreate(DxDevice *device, DxRootSignature *rootSignature, IDxcBlob *ps, IDxcBlob *vs, IDxcBlob *hs, IDxcBlob *ds)
{
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
	graphicsPipelineStateDec.pRootSignature = rootSignature->GetRootSignature();
	graphicsPipelineStateDec.InputLayout = inputLayoutDesc;
	graphicsPipelineStateDec.VS = { vs->GetBufferPointer(), vs->GetBufferSize() };
	graphicsPipelineStateDec.PS = { ps->GetBufferPointer(), ps->GetBufferSize() };
	graphicsPipelineStateDec.HS = { hs->GetBufferPointer(), hs->GetBufferSize() };
	graphicsPipelineStateDec.DS = { ds->GetBufferPointer(), ds->GetBufferSize() };
	graphicsPipelineStateDec.BlendState = blendDesc;
	graphicsPipelineStateDec.RasterizerState = rasterizerDesc;
	// 書き込むRTVの情報
	graphicsPipelineStateDec.NumRenderTargets = 1;
	graphicsPipelineStateDec.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	// 利用するトポロジタイプ
	graphicsPipelineStateDec.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
	// 色の打ち込み方設定
	graphicsPipelineStateDec.SampleDesc.Count = 1;
	graphicsPipelineStateDec.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	// 深度情報設定
	graphicsPipelineStateDec.DepthStencilState = depthStencilDesc;
	graphicsPipelineStateDec.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	// 生成
	HRESULT hr = device->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDec, IID_PPV_ARGS(&graphicsPipelineState));
	assert(SUCCEEDED(hr));
	Logger::Log("Created PSO\n");
#pragma endregion
}

ID3D12PipelineState *DxPipelineStateObject::GetPipelineState()
{
	return graphicsPipelineState.Get();
}

//UINT DxPipelineStateObject::GetRootParamIndex(const std::string& key)
//{
//	return rootSignature_->GetParamIndex(key);
//}
