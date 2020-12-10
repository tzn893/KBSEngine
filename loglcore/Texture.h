#pragma once
#include "d3dcommon.h"
#include "CopyBatch.h"

enum TEXTURE_FORMAT {
	//TEXTURE_FORMAT_RGB,
	TEXTURE_FORMAT_RGBA
};

enum TEXTURE_TYPE {
	TEXTURE_TYPE_2D
};

enum TEXTURE_FLAG {
	TEXTURE_FLAG_NONE = 0,
	TEXTURE_FLAG_ALLOW_RENDER_TARGET = 1,
	TEXTURE_FLAG_ALLOW_DEPTH_STENCIL = 1 << 1,
	TEXTURE_FLAG_ALLOW_UNORDERED_ACCESS = 1 << 2
};

//
class Texture {
public:
	Texture(size_t width,size_t height,TEXTURE_FORMAT format,
		TEXTURE_FLAG flag = TEXTURE_FLAG_NONE,D3D12_RESOURCE_STATES initState = D3D12_RESOURCE_STATE_COMMON);

	Texture(size_t width,size_t height,TEXTURE_FORMAT format,
		void* data,TEXTURE_FLAG flag  = TEXTURE_FLAG_NONE,
		D3D12_RESOURCE_STATES initState = D3D12_RESOURCE_STATE_COMMON,
		UploadBatch* batch = nullptr);

	Texture(size_t width, size_t height, TEXTURE_FORMAT format,
		TEXTURE_TYPE type, 
		D3D12_SUBRESOURCE_DATA* sub_res,
		size_t sub_res_num,
		D3D12_RESOURCE_STATES initState = D3D12_RESOURCE_STATE_COMMON,
		TEXTURE_FLAG flag = TEXTURE_FLAG_NONE
		, UploadBatch* batch = nullptr);

	ID3D12Resource* GetResource() {
		if (!isValid) return nullptr;
		return mRes.Get();
	}
	size_t GetWidth() { return width; }
	size_t GetHeight() { return height; }

	TEXTURE_TYPE GetType() { return type; }
	bool IsValid() { return isValid; }
protected:
	TEXTURE_TYPE type;
	TEXTURE_FLAG flag;
	DXGI_FORMAT  format;
	size_t width, height;

	ComPtr<ID3D12Resource> mRes;
	bool isValid;
};