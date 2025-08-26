#pragma once
// この順序でincludeすることをMicroSoftが規定している
#include <d3d12.h>
#include <dxcapi.h>

#include <string>

#include "ComPtr.h"

class DxShaderCompiler
{
public:
	static DxShaderCompiler& GetInstancxe();
	void Initialize();
	ComPtr<IDxcBlob> CompileShader(const std::string&, const wchar_t*);

private:
	DxShaderCompiler() = default;
	DxShaderCompiler(const DxShaderCompiler&) = delete;
	DxShaderCompiler& operator=(const DxShaderCompiler&) = delete;

private:

	ComPtr<IDxcUtils> dxcUtils_ = nullptr;
	ComPtr<IDxcCompiler3> dxcCompiler_ = nullptr;
	ComPtr<IDxcIncludeHandler> includeHandler_ = nullptr;
};