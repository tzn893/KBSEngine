#include "TextureManager.h"
#include "stb_image.h"
#include <filesystem>
#include <algorithm>
using namespace std::filesystem;

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

bool supportedBySTB(const path& _ext) {
	std::wstring extName = _ext.extension().wstring();
	return extName == L".png" || extName == L".bmp" || extName == L".jpg" ||
		extName == L".hdr" || extName == L".tga";
}

inline bool supported(const path& ext) {
	return supportedBySTB(ext);
}

std::tuple<void*,int,int> loadRawDataBySTB(const wchar_t* filepath,bool filp_vertically) {
	void* data = nullptr;
	int height, width;
	stbi_set_flip_vertically_on_load(filp_vertically);
	FILE* target_file = nullptr;
	if (errno_t error = _wfopen_s(&target_file, filepath, L"rb");error != 0) {
		return std::move(std::make_tuple(nullptr, -1, -1));
	}
	data = stbi_load_from_file(target_file, &width, &height, nullptr, 4);
	fclose(target_file);
	return std::move(std::make_tuple(data,width,height));
}

ManagedTexture* TextureManager::loadTexture(const wchar_t* filepath,const wchar_t* name,bool filp_vertically,UploadBatch* batch) {
	if (ManagedTexture* tex = getTextureByPath(filepath);tex != nullptr) {
		return tex;
	}
	if (ManagedTexture* tex = getTextureByName(name);tex != nullptr) {
		return nullptr;
	}

	std::filesystem::path ps(filepath);
	if (!exists(ps) || !ps.has_extension()) {
		OUTPUT_DEBUG_STRINGW((L"file " + ps.wstring() + L" doesn't exists or it is not supported\n").c_str());
		return nullptr;
	}

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
	if (supportedBySTB(ps)) {
		return loadTextureBySTB(filepath,name,filp_vertically,batch);
	}

	//the image extension name is not supportted
	return nullptr;
}

ManagedTexture* TextureManager::loadTexture(const wchar_t* filepath,size_t mipnum,const wchar_t* name,bool filp_vertically) {
	if (ManagedTexture* tex = getTextureByPath(filepath); tex != nullptr) {
		return tex;
	}
	if (ManagedTexture* tex = getTextureByName(name); tex != nullptr) {
		return nullptr;
	}

	std::filesystem::path ps(filepath);
	if (!exists(ps) || !ps.has_extension()) {
		OUTPUT_DEBUG_STRINGW((L"file " + ps.wstring() + L" doesn't exists or it is not supported\n").c_str());
		return nullptr;
	}

	static size_t id = 0;
	if (name == nullptr) {
		static std::wstring nameBuffer;
		nameBuffer = L"__unnamed_managed_texture_" + std::to_wstring(id++);
		while (getTextureByName(nameBuffer.c_str()) != nullptr) {
			nameBuffer = L"__unnamed_managed_texture_" + std::to_wstring(id++);
		}
		name = nameBuffer.c_str();
	}

	if (supportedBySTB(ps)) {
		return loadTextureBySTB(filepath, name, filp_vertically, nullptr, mipnum);
	}
	return nullptr;
}

ManagedTexture* TextureManager::loadTextureBySTB(const wchar_t* path, const wchar_t* name,bool filp_vertically,UploadBatch* batch,size_t mipnum) {

	auto[idata, iwidth, iheight] = loadRawDataBySTB(path, filp_vertically);
	if (idata == nullptr) return nullptr;
	if (mipnum == 0) {
		size_t il = iwidth < iheight ? iwidth : iheight;
		while (il != 0) {
			mipnum++, il = il >> 1;
		}
	}
	TEXTURE_FORMAT format = TEXTURE_FORMAT_RGBA;
	
	std::unique_ptr<ManagedTexture> mtexture;
	if(mipnum == 1)
		mtexture = std::make_unique<ManagedTexture>(name,path,iwidth,iheight,
			format,&idata,D3D12_RESOURCE_STATE_COMMON,batch);
	else {
		mtexture = std::make_unique<ManagedTexture>(name, path, iwidth, iheight, mipnum,
			format, &idata, D3D12_RESOURCE_STATE_COMMON);
	}
	if (!mtexture->IsValid()) {
		//fclose(target_file);
		if(idata != nullptr) free(idata);
		return nullptr;
	}

	ManagedTexture* rv = mtexture.get();
	texturesByName[name] = mtexture.get();
	texturesByPath[path] = std::move(mtexture);

	//fclose(target_file);
	return rv;
}

enum _CUBE_TEXTURE_FILE_TYPE {
	CUBE_TEXTURE_FILE_RIGHT = 0,
	CUBE_TEXTURE_FILE_LEFT = 1,
	CUBE_TEXTURE_FILE_DOWN = 2,
	CUBE_TEXTURE_FILE_UP = 3,
	CUBE_TEXTURE_FILE_FRONT = 4,
	CUBE_TEXTURE_FILE_BACK = 5
};

std::pair<bool,std::vector<std::wstring>> findCubeTextureFileInDirectory(path& path) {
	std::vector<std::wstring> output(6,std::wstring());
	directory_iterator diter{path};
	for (const directory_entry& den : diter) {
		if (den.path().stem() == L"top" && supported(den.path())) {
			output[CUBE_TEXTURE_FILE_UP] = den.path().wstring();
		}
		else if (den.path().stem() == L"bottom" && supported(den.path())) {
			output[CUBE_TEXTURE_FILE_DOWN] = den.path().wstring();
		}
		else if(den.path().stem() == L"left" && supported(den.path())){
			output[CUBE_TEXTURE_FILE_LEFT] = den.path().wstring();
		}
		else if (den.path().stem() == L"right" && supported(den.path())) {
			output[CUBE_TEXTURE_FILE_RIGHT] = den.path().wstring();
		}
		else if (den.path().stem() == L"front" && supported(den.path())) {
			output[CUBE_TEXTURE_FILE_FRONT] = den.path().wstring();
		}
		else if (den.path().stem() == L"back" && supported(den.path())) {
			output[CUBE_TEXTURE_FILE_BACK] = den.path().wstring();
		}
	}

	for (auto& item : output) {
		if (item.empty()) return std::move(std::make_pair(false,output));
	}
	return std::move(std::make_pair(true,output));
}

ManagedTexture* TextureManager::loadCubeTexture(const wchar_t* filepath,
	const wchar_t* name,bool filp_vertically,UploadBatch* batch) {
	if (ManagedTexture* tex = getTextureByPath(filepath); tex != nullptr) {
		return tex;
	}
	if (ManagedTexture* tex = getTextureByName(name); tex != nullptr) {
		return nullptr;
	}

	static size_t id = 0;
	if (name == nullptr) {
		static std::wstring nameBuffer;
		nameBuffer = L"__unnamed_managed_texture_" + std::to_wstring(id++);
		while (getTextureByName(nameBuffer.c_str()) != nullptr) {
			nameBuffer = L"__unnamed_managed_texture_" + std::to_wstring(id++);
		}
		name = nameBuffer.c_str();
	}
	
	path fp{filepath};
	if (!exists(fp) || !is_directory(status(fp))) {
		OUTPUT_DEBUG_STRINGW((L"directory " + std::wstring(filepath) + L" doesn't exists\n").c_str());
		return nullptr;
	}
	auto[valid, filepaths] = findCubeTextureFileInDirectory(fp);
	if (!valid) {
		OUTPUT_DEBUG_STRINGW((L"directory " + std::wstring(filepath) + L" is not valid\n").c_str());
		return nullptr;
	}

	D3D12_SUBRESOURCE_DATA subDatas[6];
	void* datas[6];
	int width, height;
	for (int i = 0; i <= CUBE_TEXTURE_FILE_BACK;i++) {
		void* data = nullptr;
		if (supportedBySTB(path(filepaths[i]))) {
			//this is a little bit werid but I can't come up with better expression
			auto[d, w, h] = loadRawDataBySTB(filepaths[i].c_str(), filp_vertically);
			data = d, width = w, height = h;
		}
		if (data == nullptr) {
			OUTPUT_DEBUG_STRINGW((L"fail to load file " + filepaths[i] + L"\n").c_str());
			for (int j = 0; j < i; j++) {
				free(datas[j]);
			}
			return nullptr;
		}
		subDatas[i].pData = data;
		subDatas[i].RowPitch = width * 4;
		subDatas[i].SlicePitch = height * subDatas[i].RowPitch;
		datas[i] = data;
	}

	std::unique_ptr<ManagedTexture> mtexture = std::make_unique<ManagedTexture>(
		name, filepath, width, height,
		TEXTURE_FORMAT_RGBA,
		TEXTURE_TYPE_2DCUBE,
		datas, 6,
		subDatas, 6,
		D3D12_RESOURCE_STATE_COMMON,
		batch
	);

	if (!mtexture->IsValid()) {
		for (int j = 0; j != 6; j++)
			if (datas[j] != nullptr) free(datas[j]);
	}
	
	ManagedTexture* rv = mtexture.get();
	texturesByName[name] = rv;
	texturesByPath[filepath] = std::move(mtexture);

	return rv;
}

#include "DescriptorAllocator.h"

Texture* TextureManager::getWhiteTexture() {
	if (white == nullptr) {
		char* data = (char*)malloc(4);
		data[0] = 0xff, data[1] = 0xff, data[2] = 0xff, data[3] = 0xff;
		white = std::make_unique<Texture>(1, 1, TEXTURE_FORMAT_RGBA, reinterpret_cast<void**>(&data),
			TEXTURE_FLAG_NONE);

		white->CreateShaderResourceView(gDescriptorAllocator.AllocateDescriptor());
	}
	return white.get();
}

Texture* TextureManager::getBlackTexture() {
	if (black == nullptr) {
		char* data = (char*)malloc(4);
		memset(data, 0, 4);
		black = std::make_unique<Texture>(1,1,TEXTURE_FORMAT_RGBA,reinterpret_cast<void**>(&data),
			TEXTURE_FLAG_NONE);

		black->CreateShaderResourceView(gDescriptorAllocator.AllocateDescriptor());
	}
	return black.get();
}

Texture* TextureManager::getBlueTexture() {
	if (blue == nullptr) {
		char* data = (char*)malloc(4);
		data[0] = 0, data[1] = 0, data[2] = 0xff, data[3] = 0xff;
		blue = std::make_unique<Texture>(1,1,TEXTURE_FORMAT_RGBA,reinterpret_cast<void**>(&data),
			TEXTURE_FLAG_NONE);
		blue->CreateShaderResourceView(gDescriptorAllocator.AllocateDescriptor());
	}
	return blue.get();
}

Texture* TextureManager::getNormalMapDefaultTexture() {
	if (normal == nullptr) {
		char* data = (char*)malloc(4);
		data[0] = 0x7f, data[1] = 0x7f, data[2] = 0xff, data[3] = 0xff;
		normal = std::make_unique<Texture>(1, 1, TEXTURE_FORMAT_RGBA, reinterpret_cast<void**>(&data),
			TEXTURE_FLAG_NONE);
		normal->CreateShaderResourceView(gDescriptorAllocator.AllocateDescriptor());
	}
	return normal.get();
}