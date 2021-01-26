#include "DescriptorAllocator.h"
#include "graphic.h"

DescriptorHeap::DescriptorHeap(size_t sucSize) {
	D3D12_DESCRIPTOR_HEAP_DESC desc;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	desc.NumDescriptors = sucSize;
	desc.NodeMask = 0;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	ID3D12Device* device = gGraphic.GetDevice();
	device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&mHeaps[M_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]));

	size[M_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV] = sucSize,uploadedOffsets[M_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV] = 0, allocatedOffsets[M_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV] = 0;
	mHeaps[M_DESCRIPTOR_HEAP_TYPE_DSV] = nullptr, size[M_DESCRIPTOR_HEAP_TYPE_DSV] = 0, uploadedOffsets[M_DESCRIPTOR_HEAP_TYPE_DSV] = 0, allocatedOffsets[M_DESCRIPTOR_HEAP_TYPE_DSV] = 0;
	mHeaps[M_DESCRIPTOR_HEAP_TYPE_RTV] = nullptr, size[M_DESCRIPTOR_HEAP_TYPE_RTV] = 0, uploadedOffsets[M_DESCRIPTOR_HEAP_TYPE_RTV] = 0, allocatedOffsets[M_DESCRIPTOR_HEAP_TYPE_RTV] = 0;

	handleSize[M_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV] = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	handleSize[M_DESCRIPTOR_HEAP_TYPE_DSV] = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	handleSize[M_DESCRIPTOR_HEAP_TYPE_RTV] = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
}

Descriptor DescriptorHeap::Allocate(size_t num, D3D12_DESCRIPTOR_HEAP_TYPE type) {
	ID3D12DescriptorHeap* targetHeap;
	size_t targetOffset, targetSize;
	
	M_DESCRIPTOR_HEAP_TYPE mType = mapDescriptorHeapType(type);
	if (mType < 0)
		return Descriptor();

	if (mHeaps[mType] == nullptr) {
		D3D12_DESCRIPTOR_HEAP_DESC desc;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		desc.NodeMask = 0;
		desc.NumDescriptors = defaultDSVRTVSize;
		desc.Type = type;

		size[mType] = defaultDSVRTVSize;

		ID3D12Device* device = gGraphic.GetDevice();
		HRESULT hr = device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&mHeaps[mType]));
		if (FAILED(hr)) {
			return Descriptor();
		}
	}

	if (num + allocatedOffsets[mType] + uploadedOffsets[mType] > size[mType]) {
		return Descriptor();
	}

    CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle(mHeaps[mType]->GetCPUDescriptorHandleForHeapStart());
	cpuHandle.Offset(allocatedOffsets[mType], handleSize[mType]);
	CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle(mHeaps[mType]->GetGPUDescriptorHandleForHeapStart());
	gpuHandle.Offset(allocatedOffsets[mType], handleSize[mType]);

	allocatedOffsets[mType] += num;

	return Descriptor(cpuHandle, gpuHandle, num);
}

ID3D12DescriptorHeap* DescriptorHeap::GetHeap(D3D12_DESCRIPTOR_HEAP_TYPE type) {
	switch (type) {
	case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
		return mHeaps[M_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV].Get();
	case D3D12_DESCRIPTOR_HEAP_TYPE_DSV:
		return mHeaps[M_DESCRIPTOR_HEAP_TYPE_DSV].Get();
	case D3D12_DESCRIPTOR_HEAP_TYPE_RTV:
		return mHeaps[M_DESCRIPTOR_HEAP_TYPE_RTV].Get();
	}
	return nullptr;
}

Descriptor DescriptorHeap::UploadDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE heap, D3D12_CPU_DESCRIPTOR_HANDLE handle,size_t num) {
	M_DESCRIPTOR_HEAP_TYPE mHeapType = mapDescriptorHeapType(heap);
	if (uploadedOffsets[mHeapType] + allocatedOffsets[mHeapType] + num > size[mHeapType]) {
		return Descriptor();
	}

	CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle(mHeaps[mHeapType]->GetCPUDescriptorHandleForHeapStart());
	cpuHandle.Offset(size[mHeapType] - uploadedOffsets[mHeapType] - num, handleSize[mHeapType]);
	CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle(mHeaps[mHeapType]->GetGPUDescriptorHandleForHeapStart());
	gpuHandle.Offset(size[mHeapType] - uploadedOffsets[mHeapType] - num, handleSize[mHeapType]);

	uploadedOffsets[mHeapType] += num;
	
	gGraphic.GetDevice()->CopyDescriptorsSimple(num, cpuHandle, handle, heap);
	return Descriptor(cpuHandle, gpuHandle, num);
}

void DescriptorHeap::ClearUploadedDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE type) {
	M_DESCRIPTOR_HEAP_TYPE mHeapType = mapDescriptorHeapType(type);

	uploadedOffsets[mHeapType] = 0;
}

void DescriptorHeap::ClearAllocatedDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE type) {
	M_DESCRIPTOR_HEAP_TYPE mHeapType = mapDescriptorHeapType(type);

	allocatedOffsets[mHeapType] = 0;
}

DescriptorAllocator::DescriptorAllocator() {
	allocatedSize = 0;
	handleSize = 0;
	currHeap = nullptr;
}

ComPtr<ID3D12DescriptorHeap> DescriptorAllocator::CreateHeap() {
	D3D12_DESCRIPTOR_HEAP_DESC desc;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	desc.NodeMask = 0;
	desc.NumDescriptors = heapSize;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	ComPtr<ID3D12DescriptorHeap> rv;
	gGraphic.GetDevice()->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&rv));

	return rv;
}

Descriptor DescriptorAllocator::AllocateDescriptor(size_t num) {
	if (handleSize == 0) {
		handleSize = gGraphic.GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		currHeap = CreateHeap();
	}
	if (allocatedSize + num > heapSize) {
		usedHeaps.push_back(currHeap);
		currHeap = CreateHeap();
		allocatedSize = 0;
	}

	CD3DX12_CPU_DESCRIPTOR_HANDLE rv(currHeap->GetCPUDescriptorHandleForHeapStart());
	rv.Offset(handleSize, allocatedSize);
	allocatedSize += num;

	return Descriptor(rv,num);
}

Descriptor Descriptor::Offset(D3D12_DESCRIPTOR_HEAP_TYPE type,size_t offset) {
	size_t dscStride = gGraphic.GetDescriptorHandleSize(type);
	if (offset >= num || dscStride == 0) {
		return Descriptor();
	}
	
	Descriptor newHandle = *this;
	newHandle.num -= offset;
	newHandle.cpuHandle.ptr += dscStride * offset;
	if (gpuHandle.ptr != 0) {
		newHandle.gpuHandle.ptr += dscStride * offset;
	}
	return newHandle;
}