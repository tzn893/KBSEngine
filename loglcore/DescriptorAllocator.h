#pragma once
#include "d3dcommon.h"

struct Descriptor {
	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;
	size_t num;

	Descriptor() { num = 0; cpuHandle.ptr = 0, gpuHandle.ptr = 0; }
	Descriptor(D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle,
		D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle, size_t num) :
		cpuHandle(cpuHandle),gpuHandle(gpuHandle),num(num) {}
	bool isValid() { return num != 0;}
};


class DescriptorHeap {
public:
	DescriptorHeap(size_t sucSize = 1024);
	Descriptor Allocate(size_t num = 1,D3D12_DESCRIPTOR_HEAP_TYPE type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	ID3D12DescriptorHeap* GetHeap(D3D12_DESCRIPTOR_HEAP_TYPE type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
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
	size_t offsets[3];
	size_t size[3];
	size_t handleSize[3];

	static constexpr size_t defaultDSVRTVSize = 64;
};