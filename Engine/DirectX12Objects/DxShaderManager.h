#pragma once
// この順序でincludeすることをMicroSoftが規定している
#include <d3d12.h>
#include <dxcapi.h>

#include <string>

#include "../ComPtr.h"

class DxShaderManager
{
public:
	void Initialize();
	ComPtr<IDxcBlob> CompileShader(const std::string&, const wchar_t*);

private:
	const std::string shaderDirectoryPath = "Resources/Shaders/";

	ComPtr<IDxcUtils> dxcUtils_ = nullptr;
	ComPtr<IDxcCompiler3> dxcCompiler_ = nullptr;
	ComPtr<IDxcIncludeHandler> includeHandler_ = nullptr;
};