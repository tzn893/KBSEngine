#pragma once
#include "d3dcommon.h"

class GenerateMipmapBatch {
public:
	static bool Generate(ID3D12Resource* res,D3D12_RESOURCE_STATES initState,
		size_t width,size_t height,
		size_t mipnum,D3D12_CPU_DESCRIPTOR_HANDLE* destHandles,
		D3D12_CPU_DESCRIPTOR_HANDLE* srcHandle);
private:
	static bool initialize();
};