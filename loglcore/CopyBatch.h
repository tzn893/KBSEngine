#pragma once
#include "d3dcommon.h"

class UploadBatch {
public:
	static UploadBatch Begin();
	void End(bool wait = true);

	ID3D12Resource* UploadBuffer(size_t size,void* buffer,D3D12_RESOURCE_STATES initState = D3D12_RESOURCE_STATE_COMMON);
	ID3D12Resource* UploadTexture( D3D12_SUBRESOURCE_DATA* subdata, size_t subdata_num,
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
		std::vector<D3D12_SUBRESOURCE_DATA> subResources;
	};

	std::vector<D3D12_RESOURCE_BARRIER> barriersBefore;
	std::vector<D3D12_RESOURCE_BARRIER> barriersAfter;
	std::vector<UploadBufferData> uploadBuffers;
	
	bool isAvailable;
};
