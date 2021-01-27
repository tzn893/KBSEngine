#pragma once
#include "Texture.h"
#include <map>
#include <memory>

class ManagedTexture : public Texture {
public:
	ManagedTexture(const wchar_t* name,const wchar_t* path, 
		size_t width, size_t height, TEXTURE_FORMAT format,
		void** data,D3D12_RESOURCE_STATES initState = D3D12_RESOURCE_STATE_COMMON,
		UploadBatch* batch = nullptr) :Texture(width, height, format,
			data, TEXTURE_FLAG_ALLOW_UNORDERED_ACCESS,initState, batch), name(name), pathName(path)
	{}

	ManagedTexture(const wchar_t* name, const wchar_t* path,
		size_t width, size_t height,size_t mipnum,TEXTURE_FORMAT format,
		void** data, D3D12_RESOURCE_STATES initState = D3D12_RESOURCE_STATE_COMMON) : name(name), pathName(path),
		Texture(width,height,mipnum,format,data,
		TEXTURE_FLAG_ALLOW_UNORDERED_ACCESS,initState)
	{}

	ManagedTexture(const wchar_t* name, const wchar_t* path,
		size_t width, size_t height, TEXTURE_FORMAT format,
		TEXTURE_TYPE type,
		void** original_data,
		size_t original_data_num,
		D3D12_SUBRESOURCE_DATA* sub_res,
		size_t sub_res_num,
		D3D12_RESOURCE_STATES initState = D3D12_RESOURCE_STATE_COMMON
		, UploadBatch* batch = nullptr):Texture(width,height,format,type,original_data,original_data_num,sub_res,
			sub_res_num,initState,TEXTURE_FLAG_ALLOW_UNORDERED_ACCESS, batch),name(name),pathName(path)
	{}

	const wchar_t* GetName() { return name.c_str(); }
	const wchar_t* GetPath() { return pathName.c_str(); }
private:
	std::wstring name;
	std::wstring pathName;
};

class TextureManager{
public:
	ManagedTexture* loadTexture(const wchar_t* path,const wchar_t* name = nullptr,bool filp_vertically = true,UploadBatch* batch = nullptr);
	ManagedTexture* loadTexture(const wchar_t* path, size_t mipnum, const wchar_t* name = nullptr,bool filp_vertically = true);
	ManagedTexture* getTextureByName(const wchar_t* name);
	ManagedTexture* getTextureByPath(const wchar_t* path);

	ManagedTexture* loadCubeTexture(const wchar_t* path,const wchar_t* name = nullptr,bool filp_vertically = true,UploadBatch* batch = nullptr);

	Texture*        getWhiteTexture();
	Texture*		getBlackTexture();
private:
	//the path is the main key
	std::map<std::wstring, std::unique_ptr<ManagedTexture>> texturesByPath;
	std::map<std::wstring, ManagedTexture*> texturesByName;

	ManagedTexture* loadTextureBySTB(const wchar_t* path,const wchar_t* name,bool filp_vertically,UploadBatch* batch,size_t mipnum = 1);
	std::unique_ptr<Texture> white,black;
};

inline TextureManager gTextureManager;