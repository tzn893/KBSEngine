#pragma once
#include "d3dcommon.h"

class UploadBatch {
public:
	static UploadBatch Begin();
	void End(bool wait = true);

	ID3D12Resource* UploadBuffer(size_t size,void* buffer,D3D12_RESOURCE_STATES initState = D3D12_RESOURCE_STATE_COMMON);

private:
	static bool initialize();

	static ComPtr<ID3D12GraphicsCommandList> mCmdList;
	static ComPtr<ID3D12CommandAllocator> mCmdAlloc;
	static ComPtr<ID3D12Fence> mFence;
	static HANDLE mEvent;
	static ID3D12Device* mDevice;
	static size_t fenceValue;
	static bool initialized;
	static bool isOccupied;
	
	struct UploadBufferData {
		ComPtr<ID3D12Resource> buffer;
		ComPtr<ID3D12Resource> uploadBuffer;
	};

	std::vector<D3D12_RESOURCE_BARRIER> barriersDest;
	std::vector<D3D12_RESOURCE_BARRIER> barrierTarget;
	std::vector<UploadBufferData> uploadBuffers;
	
	bool isAvailable;
};
