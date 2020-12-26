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
		VS, "vs_5_1", compileFlag, 0, &shaderByteCodeVS,&errorMessage);

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
		PS, "ps_5_1", compileFlag, 0, &shaderByteCodePS, &errorMessage);

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


ComputeShader* ShaderManager::loadComputeShader(const wchar_t* path, const char* CS,
	const wchar_t* rootSig, const wchar_t* name,
	const D3D_SHADER_MACRO* macros) {
	ComPtr<ID3DBlob> csByteCode,errorCode;

	if (getShaderByName(name) != nullptr) {
		return nullptr;
	}
	
	unsigned int compileFlag = 0;
#ifdef _DEBUG
	compileFlag = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	HRESULT hr = D3DCompileFromFile(path, macros,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		CS, "cs_5_1", compileFlag, 0,
		&csByteCode, &errorCode);
	if(FAILED(hr)){
		std::wstring message = L"fail to compile compute shader in shader name :";
		message = message + name + L",path :" + path + L",reason:\n";
		OUTPUT_DEBUG_STRINGW(message.c_str());
		if (HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND) == hr) {
			OUTPUT_DEBUG_STRING("the shader program is not founded\n")
		}
		else if (errorCode->GetBufferPointer() != nullptr) {
			OUTPUT_DEBUG_STRINGA((const char*)errorCode->GetBufferPointer());
		}
		return false;
	}

	if (name == nullptr) {
		static std::wstring namebuffer;
		static size_t index = 0;
		namebuffer = L"_unnamed_compute_shader" + std::to_wstring(index++);
		while (getComputeShaderByName(namebuffer.c_str()) != nullptr) {
			namebuffer = L"_unnamed_compute_shader" + std::to_wstring(index++);
		}
		name = namebuffer.c_str();
	}

	std::unique_ptr<ComputeShader> cs = std::make_unique<ComputeShader>(csByteCode.Get(), name, rootSig);
	ComputeShader* rv = cs.get();
	cshadersByName[name] = std::move(cs);

	return rv;
}


Shader* ShaderManager::getShaderByName(const wchar_t* name) {
	auto queryByName = shadersByName.find(name);
	if (queryByName == shadersByName.end()) {
		return nullptr;
	}
	return queryByName->second.get();
}

ComputeShader* ShaderManager::getComputeShaderByName(const wchar_t* name) {
	auto queryByName = cshadersByName.find(name);
	if (queryByName == cshadersByName.end()) {
		return nullptr;
	}
	return queryByName->second.get();
}