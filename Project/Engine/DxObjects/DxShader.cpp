#include "DxShader.h"

// ShaderComplier
#pragma comment(lib, "dxcompiler.lib")

#include <cassert>
#include <format>
#include <fstream>

#include "externals/nlohmann/json.hpp"
#include "Logger.h"
#include "StringUtility.h"

static const std::string sLoadJsonFileName = "Resources/Shaders/test.json";
static const std::string sLoadDirectory = "Resources/Shaders/";

ComPtr<IDxcUtils> DxShaderCompiler::dxcUtils_ = nullptr;
ComPtr<IDxcCompiler3> DxShaderCompiler::dxcCompiler_ = nullptr;
ComPtr<IDxcIncludeHandler> DxShaderCompiler::includeHandler_ = nullptr;
std::unordered_map<std::string, std::unique_ptr<DxShaderCompiler::ShaderGroup>> DxShaderCompiler::shaderGroups_;

//DxShaderCompiler &DxShaderCompiler::GetInstance()
//{
//	static DxShaderCompiler instance;
//	return instance;
//}

void DxShaderCompiler::Initialize()
{
	HRESULT hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils_));
	assert(SUCCEEDED(hr));
	hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler_));
	assert(SUCCEEDED(hr));
	Logger::Log("dxcCompiler Initialized\n");

	// HLSL内でインクルードするときにこの設定がいる
	hr = dxcUtils_->CreateDefaultIncludeHandler(&includeHandler_);
	assert(SUCCEEDED(hr));
}

void DxShaderCompiler::Finalize()
{
	shaderGroups_.clear();

	includeHandler_.Reset();
	dxcCompiler_.Reset();
	dxcUtils_.Reset();
}

ComPtr<IDxcBlob> DxShaderCompiler::CompileShader(const std::string &filePath, const wchar_t *profile)
{
	if (dxcUtils_ == nullptr || dxcCompiler_ == nullptr)
	{
		Logger::Log("DxShaderCompiler is not initialized.\n");
		assert(false);
		return nullptr;
	}

	std::wstring filePathW = StringUtility::ConvertToWString(sLoadDirectory + filePath);

#pragma region HLSL Loading
	Logger::Log(StringUtility::ConvertToString(std::format(L"Begin CompileShader, path:{}, profile:{}\n", filePathW, profile)));
	// hlsl Load
	ComPtr<IDxcBlobEncoding> shaderSource = nullptr;
	HRESULT hr = dxcUtils_->LoadFile(filePathW.c_str(), nullptr, &shaderSource);
	assert(SUCCEEDED(hr));
	Logger::Log("Shader Load Complete\n");

	// 内容設定
	DxcBuffer shaderSourceBuffer{};
	shaderSourceBuffer.Ptr = shaderSource->GetBufferPointer();
	shaderSourceBuffer.Size = shaderSource->GetBufferSize();
	shaderSourceBuffer.Encoding = DXC_CP_UTF8;	// UTF-8のコードであることを通知
#pragma endregion
#pragma region Compiler
	LPCWSTR arguments[] = {
		filePathW.c_str(),			// コンパイルするhlslファイル名
		L"-E", L"main",				// エントリーポイント指定
		L"-T", profile,				// ShaderProfile設定
		L"-Zi", L"-Qembed_debug",	// デバッグ用の情報埋め込み
		L"-Od",						// 最適化はしない
		L"-Zpr",					// メモリレイアウトは行優先
	};
	// Shader Compile
	ComPtr<IDxcResult> shaderResult = nullptr;
	hr = dxcCompiler_->Compile(
		&shaderSourceBuffer,		// 読み込んだファイル
		arguments,					// コンパイル時のオプション
		_countof(arguments),		// ↑の数
		includeHandler_.Get(),		// Includeとか
		IID_PPV_ARGS(&shaderResult)	// 結果
	);
	assert(SUCCEEDED(hr));

	// 問題が発生したらログに出力
	ComPtr<IDxcBlobUtf8> shaderError = nullptr;
	shaderResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&shaderError), nullptr);
	if (shaderError != nullptr && shaderError->GetStringLength() != 0)
	{
		// ログを出して停止
		Logger::Log(shaderError->GetStringPointer());
		assert(false);
	}
#pragma endregion
#pragma region Result
	ComPtr<IDxcBlob> shaderBlob = nullptr;
	hr = shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr);
	assert(SUCCEEDED(hr));
	Logger::Log(StringUtility::ConvertToString(std::format(L"Complie Succeeded, path:{}, profile:{}\n", filePathW, profile)));
#pragma endregion
	return shaderBlob;
}

DxShaderCompiler::ShaderGroup &DxShaderCompiler::CompileShaderGroup(const std::string &groupName)
{
	std::ifstream ifs(sLoadJsonFileName);
	std::unique_ptr<DxShaderCompiler::ShaderGroup> result = std::make_unique<DxShaderCompiler::ShaderGroup>();
	if (!ifs) { return *result.get(); }

	nlohmann::json readJson;
	ifs >> readJson;


	if (readJson.contains(groupName))
	{
		result->groupName = readJson[groupName]["GroupName"];
	}
	else
	{
		return *result.get();
	}

	if (readJson[groupName].contains("VS"))
	{
		result->vs = CompileShader(readJson[groupName]["VS"]["FileName"], StringUtility::ConvertToWString(readJson[groupName]["VS"]["Profile"]).c_str());
	}
	if (readJson[groupName].contains("PS"))
	{
		result->ps = CompileShader(readJson[groupName]["PS"]["FileName"], StringUtility::ConvertToWString(readJson[groupName]["PS"]["Profile"]).c_str());
	}
	if (readJson[groupName].contains("DS"))
	{
		result->ds = CompileShader(readJson[groupName]["DS"]["FileName"], StringUtility::ConvertToWString(readJson[groupName]["DS"]["Profile"]).c_str());
	}
	if (readJson[groupName].contains("GS"))
	{
		result->ds = CompileShader(readJson[groupName]["GS"]["FileName"], StringUtility::ConvertToWString(readJson[groupName]["GS"]["Profile"]).c_str());
	}
	if (readJson[groupName].contains("CS"))
	{
		result->cs = CompileShader(readJson[groupName]["CS"]["FileName"], StringUtility::ConvertToWString(readJson[groupName]["CS"]["Profile"]).c_str());
	}
	if (readJson[groupName].contains("MS"))
	{
		result->ms = CompileShader(readJson[groupName]["MS"]["FileName"], StringUtility::ConvertToWString(readJson[groupName]["MS"]["Profile"]).c_str());
	}

	shaderGroups_.emplace(groupName, std::move(result));
	return *shaderGroups_[groupName].get();
}
