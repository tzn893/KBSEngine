#include "Shader.h"


Shader* ShaderManager::loadShader( const wchar_t* path,const char* VS,const char* PS,const wchar_t* rootSigName
	, std::vector<D3D12_INPUT_ELEMENT_DESC>& inputLayout, const wchar_t* name, const D3D_SHADER_MACRO* macros) {
	if (VS == nullptr) VS = "VS";
	if (PS == nullptr) PS = "PS";

	if (name == nullptr) {
		static std::wstring name_buffer;
		static int id = 0;
		name_buffer = L"__unnamed_shader_" + std::to_wstring(id++);
		while (getShaderByName(name_buffer.c_str()) != nullptr) {
			name_buffer = L"__unnamed_shader_" + std::to_wstring(id++);
		}
		name = name_buffer.c_str();
	}

	if (Shader* query = getShaderByName(name); query != nullptr) {
		return nullptr;
	}

	unsigned int compileFlag = 0;
#ifdef _DEBUG
	compileFlag = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	ComPtr<ID3DBlob> shaderByteCodeVS,shaderByteCodePS,errorMessage;
	
	HRESULT hr = D3DCompileFromFile(path, macros, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		VS, "vs_5_0", compileFlag, 0, &shaderByteCodeVS,&errorMessage);

	if (FAILED(hr)) {
		std::wstring message = L"fail to compile vertex shader in shader name :";
		message = message + name + L",path :" + path + L",reason:\n";
		OUTPUT_DEBUG_STRINGW(message.c_str());
		if (HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND) == hr) {
			OUTPUT_DEBUG_STRING("the shader program is not founded\n")
		}
		else if (errorMessage->GetBufferPointer() != nullptr) {
			OUTPUT_DEBUG_STRINGA((const char*)errorMessage->GetBufferPointer());
		}
		return false;
	}

	hr = D3DCompileFromFile(path, macros, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		PS, "ps_5_0", compileFlag, 0, &shaderByteCodePS, &errorMessage);

	if (FAILED(hr)) {
		std::wstring message = L"fail to compile vertex shader in shader name :";
		message = message + name + L",path :" + path + L",reason:\n";
		OUTPUT_DEBUG_STRINGW(message.c_str());
		if (HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND) == hr) {
			OUTPUT_DEBUG_STRING("the shader program is not founded\n")
		}
		else if (errorMessage->GetBufferPointer() != nullptr) {
			OUTPUT_DEBUG_STRINGA((const char*)errorMessage->GetBufferPointer());
		}
		return false;
	}
	
	std::unique_ptr<Shader> shader = std::make_unique<Shader>(shaderByteCodeVS.Get(),
		shaderByteCodePS.Get(), name, path,rootSigName,inputLayout);
	shadersByName[name] = std::move(shader);

	return shadersByName[name].get();
}


Shader* ShaderManager::getShaderByName(const wchar_t* name) {
	auto queryByName = shadersByName.find(name);
	if (queryByName == shadersByName.end()) {
		return nullptr;
	}
	return queryByName->second.get();
}