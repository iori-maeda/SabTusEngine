#pragma once
#include <string>
#include <unordered_map>

#include "ComPtr.h"
#include "3D/Model.h"

class DirectXCommon;

class ModelManager
{
public:
	// シングルトンパターンで作成
	static ModelManager& GetInstace();

	void Initialize(DirectXCommon* renderContext);
	void Finalize();

	/// <summary>
	/// デフォルト設定のディレクトリから画像読み込み
	/// </summary>
	/// <param name="fileName">画像名.拡張子</param>
	/// <returns></returns>
	Model* Load(const std::string& fileName);

	Model* GetModel(const std::string& fileName);

public:

	D3D12_GPU_DESCRIPTOR_HANDLE GetSRVDescriptorGPUHandle(const std::string& key);
	
private:
	ModelManager() = default;
	ModelManager(const ModelManager&) = delete;
	ModelManager& operator=(const ModelManager&) = delete;

private:
	DirectXCommon* dxCommon_ = nullptr;

	static uint32_t modelCount;

	std::unordered_map<std::string, std::unique_ptr<Model>> models_;
};