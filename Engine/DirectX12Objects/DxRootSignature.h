#pragma once

#include <d3d12.h>
#include <vector>
#include <map>
#include <unordered_map>
#include <string>

#include "../ComPtr.h"

class DxDevice;

enum class ParamType
{
	PixelCBuffer,
	PixelTex,
	VertexCbuffer,
	VertexTex
};

class DxRootSignature
{
public:
	/// <summary>
	/// Basic3D用のデフォルト設定
	/// </summary>
	void DefaultSettings();

	/// <summary>
	/// ルートシグネチャ生成
	/// これより前にDefaultSettingsか手動でパラメータを追加しておくこと
	/// </summary>
	/// <param name="device">デバイス</param>
	void Create(DxDevice *device);

	/// <summary>
	/// ルートパラメータ追加
	/// </summary>
	/// <param name="type"></param>
	/// <param name="registerIndex">バインドするレジスタ</param>
	void AddRootParameter(ParamType type, UINT bindRegister);

	/// <summary>
	/// テンプレディスクリプタレンジ生成
	/// </summary>
	void CreateDescriptorRange();

	/// <summary>
	/// テンプレサンプラー生成
	/// </summary>
	void CreateStaticSamplers();

	/// <summary>
	/// 現状テクスチャ専用のディスクリプターレンジのセット
	/// </summary>
	/// <param name="baseRegsterIndex">レジスタ開始インデックス</param>
	/// <param name="numIndexies">確保レジスタ数</param>
	void SetDescriptorRange(UINT baseRegsterIndex, UINT numIndexies);

	ID3D12RootSignature *GetRootSignature();
	std::vector<D3D12_ROOT_PARAMETER> GetRootParameters();
private:

	ComPtr<ID3D12RootSignature> rootSignature_ = nullptr;

	// texture用
	std::vector<D3D12_DESCRIPTOR_RANGE> descriptorRange_;
	std::vector<D3D12_STATIC_SAMPLER_DESC> staticSamplers_;

	struct ResourceData
	{
		D3D12_ROOT_PARAMETER param;
		ComPtr<ID3D12Resource> resource;
	};

	// パラメタコンテナ
	std::unordered_map<std::string, ResourceData> rootParameters_;
	UINT paramIndex_ = 0;
};