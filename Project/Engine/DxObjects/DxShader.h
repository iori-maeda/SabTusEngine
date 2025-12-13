#pragma once
// この順序でincludeすることをMicroSoftが規定している
#include <d3d12.h>
#include <dxcapi.h>
#include <string>
#include <unordered_map>
#include <memory>

#include "ComPtr.h"

class DxShaderCompiler
{
public:
	typedef ComPtr<IDxcBlob> ShaderPtr;

	struct ShaderGroup
	{
		std::string groupName;
		ShaderPtr vs = nullptr;
		ShaderPtr ps = nullptr;
		ShaderPtr ds = nullptr;
		ShaderPtr gs = nullptr;
		ShaderPtr cs = nullptr;
		ShaderPtr ms = nullptr;
	};

public:
	//static DxShaderCompiler& GetInstance();
	static void Initialize();
	static void Finalize();

	static ComPtr<IDxcBlob> CompileShader(const std::string &, const wchar_t *);
	static DxShaderCompiler::ShaderGroup &CompileShaderGroup(const std::string &groupName);

private:
	DxShaderCompiler() = default;
	DxShaderCompiler(const DxShaderCompiler &) = delete;
	DxShaderCompiler &operator=(const DxShaderCompiler &) = delete;

private:
	static ComPtr<IDxcUtils> dxcUtils_;
	static ComPtr<IDxcCompiler3> dxcCompiler_;
	static ComPtr<IDxcIncludeHandler> includeHandler_;
	static std::unordered_map<std::string, std::unique_ptr<DxShaderCompiler::ShaderGroup>> shaderGroups_;
};