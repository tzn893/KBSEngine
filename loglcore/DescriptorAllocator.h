#pragma once
#include "d3dcommon.h"

struct Descriptor {
	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;
	size_t num;

	Descriptor() { num = 0; cpuHandle.ptr = 0, gpuHandle.ptr = 0; }
	Descriptor(D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle,
		D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle, size_t num = 1) :
		cpuHandle(cpuHandle),gpuHandle(gpuHandle),num(num) {}
	Descriptor(D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle,size_t num = 1):
		cpuHandle(cpuHandle), num(num) {
		gpuHandle.ptr = 0;
	}
	bool isValid() { return num != 0;}
};


class DescriptorHeap {
public:
	DescriptorHeap(size_t sucSize = 1024);
	Descriptor Allocate(size_t num = 1,D3D12_DESCRIPTOR_HEAP_TYPE type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	ID3D12DescriptorHeap* GetHeap(D3D12_DESCRIPTOR_HEAP_TYPE type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	Descriptor UploadDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE heap, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, size_t num = 1);
	void ClearUploadedDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE heap);
	void ClearAllocatedDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE heap);
private:
	enum M_DESCRIPTOR_HEAP_TYPE {
		M_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV = 0,
		M_DESCRIPTOR_HEAP_TYPE_RTV = 1,
		M_DESCRIPTOR_HEAP_TYPE_DSV = 2
	};

	M_DESCRIPTOR_HEAP_TYPE mapDescriptorHeapType(D3D12_DESCRIPTOR_HEAP_TYPE type) {
		switch (type) {
		case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
			return M_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		case D3D12_DESCRIPTOR_HEAP_TYPE_DSV:
			return M_DESCRIPTOR_HEAP_TYPE_DSV;
		case D3D12_DESCRIPTOR_HEAP_TYPE_RTV:
			return M_DESCRIPTOR_HEAP_TYPE_RTV;
		}
		return (M_DESCRIPTOR_HEAP_TYPE)-1;
	}

	ComPtr<ID3D12DescriptorHeap> mHeaps[3];
	size_t uploadedOffsets[3];
	size_t allocatedOffsets[3];
	size_t size[3];
	size_t handleSize[3];

	static constexpr size_t defaultDSVRTVSize = 8;
};

class DescriptorAllocator {
public:
	DescriptorAllocator();
	~DescriptorAllocator() { currHeap = nullptr; usedHeaps.clear(); }
	D3D12_CPU_DESCRIPTOR_HANDLE AllocateDescriptor(size_t num = 1);
private:
	ComPtr<ID3D12DescriptorHeap> CreateHeap();

	size_t allocatedSize;
	static constexpr size_t heapSize = 1024;
	size_t handleSize;
	ComPtr<ID3D12DescriptorHeap> currHeap;
	std::vector<ComPtr<ID3D12DescriptorHeap>> usedHeaps;
};

inline DescriptorAllocator gDescriptorAllocator;