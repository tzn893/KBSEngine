#include "Texture.h"
#include "graphic.h"

static size_t  getFormatElementSize(TEXTURE_FORMAT format) {
	switch (format) {
	
	case TEXTURE_FORMAT_FLOAT4:
		return 4;
	case TEXTURE_FORMAT_FLOAT2:
		return 2;
	case TEXTURE_FORMAT_RGBA:
		return 4;
	case TEXTURE_FORMAT_FLOAT:
		return 1;
	}
	return 0;
}

static DXGI_FORMAT getDXGIFormatFromTextureFormat(TEXTURE_FORMAT format) {
	switch (format) {
	case TEXTURE_FORMAT_FLOAT4:
		return DXGI_FORMAT_R32G32B32A32_FLOAT;
	case TEXTURE_FORMAT_FLOAT2:
		return DXGI_FORMAT_R32G32_FLOAT;
	case TEXTURE_FORMAT_RGBA:
		return DXGI_FORMAT_R8G8B8A8_UNORM;
	case TEXTURE_FORMAT_FLOAT:
		return DXGI_FORMAT_R32_FLOAT;
	case TEXTURE_FORMAT_DEPTH_STENCIL:
		return DXGI_FORMAT_D24_UNORM_S8_UINT;
	}
	return DXGI_FORMAT(0);
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

static D3D12_RESOURCE_DIMENSION getResourceDimensionFromResourceType(TEXTURE_TYPE type) {
	switch (type) {
	case TEXTURE_TYPE_2D:
		return D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	case TEXTURE_TYPE_2DCUBE:
		return D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	}
	return D3D12_RESOURCE_DIMENSION(-1);
}

Texture::Texture(size_t width, size_t height, TEXTURE_FORMAT format,
	TEXTURE_FLAG flag , D3D12_RESOURCE_STATES initState,D3D12_CLEAR_VALUE* clearValue) {
		this->width = width;
		this->height = height;
		this->flag = flag;

		this->format = getDXGIFormatFromTextureFormat(format);
		this->type = TEXTURE_TYPE_2D;

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
			clearValue, IID_PPV_ARGS(&mRes)
		);
		if (FAILED(hr)) {
			isValid = false;
			return;
		}

		isValid = true;
}

Texture::Texture(size_t width, size_t height, TEXTURE_FORMAT format,
	void** data, TEXTURE_FLAG flag,
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

		UploadTextureResource resource;
		resource.original_buffer.push_back(*data);

			D3D12_SUBRESOURCE_DATA subData;
			subData.pData = *data;
			subData.RowPitch = width * elementSize;
			subData.SlicePitch = height * subData.RowPitch;
			*data = nullptr;

		resource.subres.push_back(subData);
		
		if(batch != nullptr)
			mRes = batch->UploadTexture(resource,rDesc, initState);
		else {
			UploadBatch mbatch = UploadBatch::Begin();
			mRes = mbatch.UploadTexture(resource,rDesc, initState);
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
	void** orignal_data,
	size_t orignal_data_num,
	D3D12_SUBRESOURCE_DATA* sub_res,
	size_t sub_res_num,
	D3D12_RESOURCE_STATES initState,
	TEXTURE_FLAG flag
	, UploadBatch* batch) {
	this->width = width;
	this->height = height;
	this->flag = flag;
	this->type = type;

	this->format = getDXGIFormatFromTextureFormat(format);
	D3D12_RESOURCE_DESC rDesc;
	rDesc.Alignment = 0;
	rDesc.DepthOrArraySize = sub_res_num;
	rDesc.Dimension = getResourceDimensionFromResourceType(type);
	rDesc.Flags = getResourceFlagFromTextureFlag(flag);
	rDesc.Format = this->format;
	rDesc.Height = height;
	rDesc.Width = width;
	rDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	rDesc.MipLevels = 1;
	rDesc.SampleDesc.Count = 1;
	rDesc.SampleDesc.Quality = 0;

	UploadTextureResource resource;
	resource.subres.insert(resource.subres.begin(),
		sub_res, sub_res + sub_res_num);
	resource.original_buffer.insert(resource.original_buffer.begin(),
		orignal_data,orignal_data + orignal_data_num);
	for (int i = 0; i != orignal_data_num; i++)
		orignal_data[i] = nullptr;

	size_t elementSize = getFormatElementSize(format);

	if (batch != nullptr)
		mRes = batch->UploadTexture(resource, rDesc, initState);
	else {
		UploadBatch mbatch = UploadBatch::Begin();
		mRes = mbatch.UploadTexture(resource, rDesc, initState);
		mbatch.End();
	}
	if (mRes == nullptr) {
		isValid = false;
		return;
	}
	isValid = true;

}

template<typename TARGET>
static TARGET GetMostPossibleDimension(TEXTURE_TYPE type) {
	if constexpr (std::is_same<TARGET,D3D12_SRV_DIMENSION>::value) {
		switch (type) {
		case TEXTURE_TYPE_2D:
			return D3D12_SRV_DIMENSION_TEXTURE2D;
		case TEXTURE_TYPE_2DCUBE:
			return D3D12_SRV_DIMENSION_TEXTURECUBE;
		}
	}
	else if constexpr (std::is_same<TARGET,D3D12_RTV_DIMENSION>::value) {
		switch (type) {
		case TEXTURE_TYPE_2D:
			return D3D12_RTV_DIMENSION_TEXTURE2D;
		case TEXTURE_TYPE_2DCUBE:
			return D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
		}
	}
	else if constexpr (std::is_same<TARGET,D3D12_DSV_DIMENSION>::value) {
		switch (type) {
		case TEXTURE_TYPE_2D:
			return D3D12_DSV_DIMENSION_TEXTURE2D;
		case TEXTURE_TYPE_2DCUBE:
			return D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
		}
	}
	else if constexpr (std::is_same<TARGET,D3D12_UAV_DIMENSION>::value) {
		switch (type) {
		case TEXTURE_TYPE_2D:
			return D3D12_UAV_DIMENSION_TEXTURE2D;
		case TEXTURE_TYPE_2DCUBE:
			return D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
		}
	}


	return TARGET(0);
}

void Texture::CreateShaderResourceView(Descriptor descriptor,D3D12_SHADER_RESOURCE_VIEW_DESC* srv) {
	ID3D12Device* device = gGraphic.GetDevice();
	if(srv != nullptr)
		device->CreateShaderResourceView(mRes.Get(),srv,descriptor.cpuHandle);
	else {
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = GetFormat();
		srvDesc.ViewDimension = GetMostPossibleDimension<D3D12_SRV_DIMENSION>(type);
		switch (type) {
		case TEXTURE_TYPE_2D:
			srvDesc.Texture2D.MostDetailedMip = 0;
			srvDesc.Texture2D.MipLevels = -1;
			srvDesc.Texture2D.ResourceMinLODClamp = 0.f;
		case TEXTURE_TYPE_2DCUBE:
			srvDesc.TextureCube.MipLevels = -1;
			srvDesc.TextureCube.MostDetailedMip = 0;
			srvDesc.TextureCube.ResourceMinLODClamp = 0.f;
		}
		device->CreateShaderResourceView(mRes.Get(), &srvDesc, descriptor.cpuHandle);
	}
	mSRV = descriptor;
}

void Texture::CreateRenderTargetView(Descriptor descriptor,D3D12_RENDER_TARGET_VIEW_DESC* rtv) {
	ID3D12Device* device = gGraphic.GetDevice();
	if (!(flag & TEXTURE_FLAG_ALLOW_RENDER_TARGET)) {
		return;//the texture doesn't allowed creating render target view
	}
	if (rtv != nullptr) {
		device->CreateRenderTargetView(mRes.Get(), rtv, descriptor.cpuHandle);
	}
	else {
		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
		rtvDesc.ViewDimension = GetMostPossibleDimension<D3D12_RTV_DIMENSION>(type);
		rtvDesc.Format = GetFormat();
		switch (type) {
		case TEXTURE_TYPE_2D:
			rtvDesc.Texture2D = { 0,0 };
		case TEXTURE_TYPE_2DCUBE:
			rtvDesc.Texture2DArray.ArraySize = 6;
			rtvDesc.Texture2DArray.FirstArraySlice = 0;
			rtvDesc.Texture2DArray.MipSlice = 0;
			rtvDesc.Texture2DArray.PlaneSlice = 0;
		}
		device->CreateRenderTargetView(mRes.Get(), &rtvDesc, descriptor.cpuHandle);
	}

	mRTV = descriptor;
}

void Texture::CreateDepthStencilView(Descriptor descriptor,D3D12_DEPTH_STENCIL_VIEW_DESC* dsv) {
	ID3D12Device* device = gGraphic.GetDevice();
	if (!(flag & TEXTURE_FLAG_ALLOW_DEPTH_STENCIL)) {
		return;
	}
	if (dsv != nullptr) {
		device->CreateDepthStencilView(mRes.Get(), dsv, descriptor.cpuHandle);
	}
	else {
		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
		dsvDesc.ViewDimension = GetMostPossibleDimension<D3D12_DSV_DIMENSION>(type);
		dsvDesc.Format = GetFormat();
		dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
		switch (type) {
		case TEXTURE_TYPE_2D:
			dsvDesc.Texture2D.MipSlice = 0;
		case TEXTURE_TYPE_2DCUBE:
			dsvDesc.Texture2DArray.ArraySize = 6;
			dsvDesc.Texture2DArray.FirstArraySlice = 0;
			dsvDesc.Texture2DArray.MipSlice = 0;
		}
		device->CreateDepthStencilView(mRes.Get(), &dsvDesc, descriptor.cpuHandle);
	}

	mDSV = descriptor;
}

void Texture::CreateUnorderedAccessView(Descriptor descriptor,D3D12_UNORDERED_ACCESS_VIEW_DESC* uav) {
	ID3D12Device* device = gGraphic.GetDevice();
	if (!(flag & TEXTURE_FLAG_ALLOW_UNORDERED_ACCESS)) {
		return;
	}
	if (uav != nullptr) {
		device->CreateUnorderedAccessView(mRes.Get(), nullptr, uav, descriptor.cpuHandle);
	}
	else {
		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
		uavDesc.ViewDimension = GetMostPossibleDimension<D3D12_UAV_DIMENSION>(type);
		uavDesc.Format = GetFormat();
		switch (type) {
		case TEXTURE_TYPE_2D:
			uavDesc.Texture2D = { 0,0 };
		case TEXTURE_TYPE_2DCUBE:
			uavDesc.Texture2DArray.ArraySize = 6;
			uavDesc.Texture2DArray.FirstArraySlice = 0;
			uavDesc.Texture2DArray.MipSlice = 0;
			uavDesc.Texture2DArray.PlaneSlice = 0;
		}
		device->CreateUnorderedAccessView(mRes.Get(), nullptr,&uavDesc, descriptor.cpuHandle);
	}
	mUAV = descriptor;
}