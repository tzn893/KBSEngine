#pragma once
#include "d3dcommon.h"

class UploadBatch {
public:
	static UploadBatch Begin();
	void End(bool wait = true);

	ID3D12Resource* UploadBuffer(size_t size,void* buffer,D3D12_RESOURCE_STATES initState = D3D12_RESOURCE_STATE_COMMON);

private:
	static bool initialize();

	struct UploadBufferData {
		ComPtr<ID3D12Resource> buffer;
		ComPtr<ID3D12Resource> uploadBuffer;
	};

	std::vector<D3D12_RESOURCE_BARRIER> barriersDest;
	std::vector<D3D12_RESOURCE_BARRIER> barrierTarget;
	std::vector<UploadBufferData> uploadBuffers;
	
	bool isAvailable;
};
