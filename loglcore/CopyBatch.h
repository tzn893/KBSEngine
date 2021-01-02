#pragma once
#include "d3dcommon.h"

struct UploadTextureResource {
	std::vector<D3D12_SUBRESOURCE_DATA> subres;
	std::vector<void*>  original_buffer;
};

class UploadBatch {
public:
	static UploadBatch Begin();
	void End(bool wait = true);

	~UploadBatch() {
		if (isAvailable) End(true);
		isAvailable = false;
	}

	ID3D12Resource* UploadBuffer(size_t size,void* buffer,D3D12_RESOURCE_STATES initState = D3D12_RESOURCE_STATE_COMMON);
	ID3D12Resource* UploadTexture(UploadTextureResource& resource,
		D3D12_RESOURCE_DESC desc,D3D12_RESOURCE_STATES initState = D3D12_RESOURCE_STATE_COMMON);
private:
	static bool initialize();

	enum UPLOAD_BUFFER_TYPE {
		UPLOAD_BUFFER_TYPE_CONSTANT,
		UPLOAD_BUFFER_TYPE_TEXTURE
	};

	struct UploadBufferData {
		ComPtr<ID3D12Resource> buffer;
		ComPtr<ID3D12Resource> uploadBuffer;
		
		UPLOAD_BUFFER_TYPE type;
		UploadTextureResource resource;
	};

	std::vector<D3D12_RESOURCE_BARRIER> barriersBefore;
	std::vector<D3D12_RESOURCE_BARRIER> barriersAfter;
	std::vector<UploadBufferData> uploadBuffers;
	
	bool isAvailable;
};
