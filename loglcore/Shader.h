#pragma once
#include "d3dcommon.h"
#include <map>
#include <memory>

struct Shader {
	ComPtr<ID3DBlob> shaderByteCodeVS;
	size_t shaderByteSizeVS;
	ComPtr<ID3DBlob> shaderByteCodePS;
	size_t shaderByteSizePS;
	std::wstring name;
	std::wstring path;
	std::wstring rootSignatureName;

	std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout;

	Shader(ID3DBlob* shaderByteCodeVS,ID3DBlob* shaderByteCodePS,const wchar_t* name,const wchar_t* path,const wchar_t* rootSig,
		std::vector<D3D12_INPUT_ELEMENT_DESC>& inputLayout):
		name(name),path(path),shaderByteCodeVS(shaderByteCodeVS),
		rootSignatureName(rootSig),shaderByteCodePS(shaderByteCodePS),
		inputLayout(inputLayout){
		if (shaderByteCodeVS != nullptr) {
			shaderByteSizeVS = shaderByteCodeVS->GetBufferSize();
		}
		if (shaderByteCodePS != nullptr) {
			shaderByteSizePS = shaderByteCodePS->GetBufferSize();
		}
	}

	~Shader() { shaderByteCodeVS = nullptr, shaderByteCodePS = nullptr; }
};


struct ComputeShader {
	ComPtr<ID3DBlob> shaderByteCodeCS;
	size_t shaderByteSizeCS;

	std::wstring name;
	std::wstring rootSignatureName;

	ComputeShader(ID3DBlob* shaderByteCodeCS, const wchar_t* name, const wchar_t* rootSig) :name(name),rootSignatureName(rootSig),
	shaderByteCodeCS(shaderByteCodeCS){
		shaderByteSizeCS = shaderByteCodeCS->GetBufferSize();
	}
	~ComputeShader() { shaderByteCodeCS = nullptr; }
};

class ShaderManager {
public:
	Shader* loadShader(const wchar_t* path,const char* VS,const char* PS,const wchar_t* rootSigName,
		std::vector<D3D12_INPUT_ELEMENT_DESC>& inputLayout,
			const wchar_t* name = nullptr,const D3D_SHADER_MACRO* macros = nullptr);

	ComputeShader* loadComputeShader(const wchar_t* path,const char* CS,
		const wchar_t* rootSig,const wchar_t* name = nullptr,
		const D3D_SHADER_MACRO* macros = nullptr);

	Shader* getShaderByName(const wchar_t* name);
	ComputeShader* getComputeShaderByName(const wchar_t* name);
	~ShaderManager() { shadersByName.clear(); }
private:

	std::map<std::wstring, std::unique_ptr<Shader>> shadersByName;
	std::map<std::wstring, std::unique_ptr<ComputeShader>> cshadersByName;
	//std::map<std::wstring, Shader*> shadersByName;
	static const char* shaderTargets[];
};

inline ShaderManager gShaderManager;

