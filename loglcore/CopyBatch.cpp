#include "CopyBatch.h"

#include "graphic.h"

bool UploadBatch::initialized = false;
bool UploadBatch::isOccupied = false;
size_t UploadBatch::fenceValue = 0;

bool UploadBatch::initialize() {
	ID3D12Device* device = gGraphic.GetDevice();
	mDevice = device;

	HRESULT hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COPY, IID_PPV_ARGS(&mCmdAlloc));
	if (FAILED(hr)) {
		OUTPUT_DEBUG_STRING("fail to create command allocator for upload batch\n");
		return false;
	}
	hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COPY, mCmdAlloc.Get(), nullptr, IID_PPV_ARGS(&mCmdList));
	if (FAILED(hr)) {
		OUTPUT_DEBUG_STRING("fail to create command list for upload batch\n");
		return false;
	}

	mEvent = CreateEventEx(NULL, NULL, NULL, EVENT_ALL_ACCESS);
	if (mEvent == NULL) {
		OUTPUT_DEBUG_STRING("ERROR : fail to create fence event\n");
		return false;
	}
	hr = mDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mFence));
	if (FAILED(hr)) {
		OUTPUT_DEBUG_STRING("ERROR : fail to create fence\n");
		return false;
	}

	return true;
}


UploadBatch UploadBatch::Begin() {
	if (!initialized) {
		if (!initialize()) {
			UploadBatch batch;
			batch.isAvailable = false;
			return batch;
		}
		initialized = true;
	}

	if (isOccupied) {
		UploadBatch batch;
		batch.isAvailable = false;
		return batch;
	}
	else {
		UploadBatch batch;
		batch.isAvailable = true;
		isOccupied = true;
		return batch;
	}
}

void UploadBatch::End(bool wait) {
	//currently we only support upload immediately
	wait = true;

	if (!isAvailable) return;

	isOccupied = false;
	isAvailable = false;

	ID3D12CommandQueue* cmdQueue = gGraphic.GetCommandQueue();

	mCmdAlloc->Reset();
	mCmdList->Reset(mCmdAlloc.Get(), nullptr);

	mCmdList->ResourceBarrier(barriersDest.size(), barriersDest.data());

	for (auto& buffer : uploadBuffers) {
		mCmdList->CopyResource(buffer.buffer.Get(), buffer.uploadBuffer.Get());
	}

	mCmdList->ResourceBarrier(barrierTarget.size(), barrierTarget.data());

	mCmdList->Close();

	uploadBuffers.clear();
	

	if (wait) {
		ID3D12CommandList* toExcute[] = {mCmdList.Get()};
		cmdQueue->ExecuteCommandLists(_countof(toExcute), toExcute);

		fenceValue++;
		cmdQueue->Signal(mFence.Get(), fenceValue);
		if (mFence->GetCompletedValue() < fenceValue) {
			if (FAILED(mFence->SetEventOnCompletion(fenceValue, mEvent))) {
				OUTPUT_DEBUG_STRING("fail to set wait event for completion\n");
				exit(-1);
			}
			WaitForSingleObject(mEvent, INFINITE);
		}
	}
}

ID3D12Resource* UploadBatch::UploadBuffer(size_t size,void* buffer,D3D12_RESOURCE_STATES initState) {
	if (!initialized || !isAvailable)
		return nullptr;

	UploadBufferData data;

	mDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(size),
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&data.buffer)
	);

	mDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(size),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&data.uploadBuffer)
	);

	void* bufferWriter;
	data.uploadBuffer->Map(0, nullptr, &bufferWriter);
	memcpy(bufferWriter, buffer, size);
	data.uploadBuffer->Unmap(0, nullptr);

	barriersDest.push_back(CD3DX12_RESOURCE_BARRIER::Transition(
		data.buffer.Get(),D3D12_RESOURCE_STATE_COMMON,
		D3D12_RESOURCE_STATE_COPY_DEST));
	if (initState != D3D12_RESOURCE_STATE_COPY_DEST) {
		barrierTarget.push_back(CD3DX12_RESOURCE_BARRIER::Transition(
			data.buffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST,
			initState
		));
	}

	uploadBuffers.push_back(data);
}