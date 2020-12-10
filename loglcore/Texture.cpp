#include "Texture.h"
#include "graphic.h"

static size_t  getFormatElementSize(TEXTURE_FORMAT format) {
	switch (format) {
	//case TEXTURE_FORMAT_RGB:
		//return 3;
	case TEXTURE_FORMAT_RGBA:
		return 4;
	}
}

static DXGI_FORMAT getDXGIFormatFromTextureFormat(TEXTURE_FORMAT format) {
	switch (format) {
	//case TEXTURE_FORMAT_RGB:
		//return DXGI_FORMAT_R8G8B8A8_UNORM;
	case TEXTURE_FORMAT_RGBA:
		return DXGI_FORMAT_R8G8B8A8_UNORM;
	}
}

static D3D12_RESOURCE_FLAGS getResourceFlagFromTextureFlag(TEXTURE_FLAG flag) {
	D3D12_RESOURCE_FLAGS result = D3D12_RESOURCE_FLAG_NONE;
	if (flag & TEXTURE_FLAG_ALLOW_RENDER_TARGET) {
		result |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	}
	if (flag & TEXTURE_FLAG_ALLOW_DEPTH_STENCIL) {
		result |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	}
	if (flag & TEXTURE_FLAG_ALLOW_UNORDERED_ACCESS) {
		result |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	}
	return result;
}

Texture::Texture(size_t width, size_t height, TEXTURE_FORMAT format,
	TEXTURE_FLAG flag , D3D12_RESOURCE_STATES initState) {
		this->width = width;
		this->height = height;
		this->flag = flag;

		this->format = getDXGIFormatFromTextureFormat(format);
		D3D12_RESOURCE_DESC rDesc;
		rDesc.Alignment = 0;
		rDesc.DepthOrArraySize = 1;
		rDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		rDesc.Flags = getResourceFlagFromTextureFlag(flag);
		rDesc.Format = this->format;
		rDesc.Height = height;
		rDesc.Width = width;
		rDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		rDesc.MipLevels = 1;
		rDesc.SampleDesc.Count = 1;
		rDesc.SampleDesc.Quality = 0;

		ID3D12Device* device = gGraphic.GetDevice();
		HRESULT hr = device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&rDesc, initState,
			nullptr, IID_PPV_ARGS(&mRes)
		);
		if (FAILED(hr)) {
			isValid = false;
			return;
		}

		isValid = true;
}

Texture::Texture(size_t width, size_t height, TEXTURE_FORMAT format,
	void* data, TEXTURE_FLAG flag,
	D3D12_RESOURCE_STATES initState,
	UploadBatch* batch) {
	this->width = width;
	this->height = height;
	this->flag = flag;
	
	this->format = getDXGIFormatFromTextureFormat(format);
	D3D12_RESOURCE_DESC rDesc;
	rDesc.Alignment = 0;
	rDesc.DepthOrArraySize = 1;
	rDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	rDesc.Flags = getResourceFlagFromTextureFlag(flag);
	rDesc.Format = this->format;
	rDesc.Height = height;
	rDesc.Width = width;
	rDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	rDesc.MipLevels = 1;
	rDesc.SampleDesc.Count = 1;
	rDesc.SampleDesc.Quality = 0;

	if (data == nullptr) {
		isValid = false;
		return;
	}
	if (data != nullptr) {
		size_t elementSize = getFormatElementSize(format);

		D3D12_SUBRESOURCE_DATA subData;
		subData.pData = data;
		subData.RowPitch = width * elementSize;
		subData.SlicePitch = height * subData.RowPitch;
		if(batch != nullptr)
			mRes = batch->UploadTexture(&subData,1, rDesc, initState);
		else {
			UploadBatch mbatch = UploadBatch::Begin();
			mRes = mbatch.UploadTexture(&subData,1, rDesc, initState);
			mbatch.End();
		}
		if (mRes == nullptr) {
			isValid = false;
			return;
		}
	}
	isValid = true;

}

Texture::Texture(size_t width, size_t height, TEXTURE_FORMAT format,
	TEXTURE_TYPE type,
	D3D12_SUBRESOURCE_DATA* sub_res,
	size_t sub_res_num,
	D3D12_RESOURCE_STATES initState,
	TEXTURE_FLAG flag
	, UploadBatch* batch) {
	this->width = width;
	this->height = height;
	this->flag = flag;

	this->format = getDXGIFormatFromTextureFormat(format);
	D3D12_RESOURCE_DESC rDesc;
	rDesc.Alignment = 0;
	rDesc.DepthOrArraySize = 1;
	rDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	rDesc.Flags = getResourceFlagFromTextureFlag(flag);
	rDesc.Format = this->format;
	rDesc.Height = height;
	rDesc.Width = width;
	rDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	rDesc.MipLevels = 1;
	rDesc.SampleDesc.Count = 1;
	rDesc.SampleDesc.Quality = 0;

	
	size_t elementSize = getFormatElementSize(format);

	if (batch != nullptr)
		mRes = batch->UploadTexture(sub_res,sub_res_num, rDesc, initState);
	else {
		UploadBatch mbatch = UploadBatch::Begin();
		mRes = mbatch.UploadTexture(sub_res,sub_res_num, rDesc, initState);
		mbatch.End();
	}
	if (mRes == nullptr) {
		isValid = false;
		return;
	}
	isValid = true;

}