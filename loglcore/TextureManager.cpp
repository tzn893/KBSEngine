#include "TextureManager.h"

#include "ThridParty/stb/stb_image.h"

ManagedTexture* TextureManager::getTextureByName(const wchar_t* name) {
	auto query = texturesByName.find(name);
	if (query == texturesByName.end()) {
		return nullptr;
	}
	return query->second;
}
ManagedTexture* TextureManager::getTextureByPath(const wchar_t* path) {
	auto query = texturesByPath.find(path);
	if (query == texturesByPath.end()) {
		return nullptr;
	}
	return query->second.get();
}


ManagedTexture* TextureManager::loadTexture(const wchar_t* path,const wchar_t* name,bool filp_vertically,UploadBatch* batch) {
	if (ManagedTexture* tex = getTextureByPath(path);tex != nullptr) {
		return tex;
	}
	if (ManagedTexture* tex = getTextureByName(name);tex != nullptr) {
		return nullptr;
	}

	std::wstring path_str(path);
	size_t index = path_str.find_last_of(L'.');
	if (index >= path_str.size()) {
		//invalid path name can't find any extension
		return nullptr;
	}
	std::wstring extName = path_str.substr(index, path_str.size() - index);

	static size_t id = 0;
	if (name == nullptr) {
		static std::wstring nameBuffer;
		nameBuffer = L"__unnamed_managed_texture_" + std::to_wstring(id++);
		while (getTextureByName(nameBuffer.c_str()) != nullptr) {
			nameBuffer = L"__unnamed_managed_texture_" + std::to_wstring(id++);
		}
		name = nameBuffer.c_str();
	}

	//check the avaliable extension names
	if (extName == L".png" || extName == L".bmp" || extName == L".jpg" ||
		extName == L".hdr" || extName == L".tga") {
		return loadTextureBySTB(path,name,filp_vertically,batch);
	}

	//the image extension name is not supportted
	return nullptr;
}

ManagedTexture* TextureManager::loadTextureBySTB(const wchar_t* path, const wchar_t* name,bool filp_vertically,UploadBatch* batch) {
	FILE* target_file = nullptr;
	errno_t error = _wfopen_s(&target_file,path,L"rb");
	if (error != 0) return nullptr;

	stbi_set_flip_vertically_on_load(filp_vertically);

	int iwidth, iheight;
	void* idata = stbi_load_from_file(target_file, &iwidth, &iheight, nullptr, 4);

	if (idata == nullptr) {
		fclose(target_file);
		return nullptr;
	}

	TEXTURE_FORMAT format = TEXTURE_FORMAT_RGBA;

	std::unique_ptr<ManagedTexture> mtexture = std::make_unique<ManagedTexture>(name,path,iwidth,iheight,
		format,&idata,D3D12_RESOURCE_STATE_COMMON,batch);
	if (!mtexture->IsValid()) {
		fclose(target_file);
		free(idata);
		return nullptr;
	}

	ManagedTexture* rv = mtexture.get();
	texturesByName[name] = mtexture.get();
	texturesByPath[path] = std::move(mtexture);

	fclose(target_file);
	return rv;
}