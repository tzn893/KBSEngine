#pragma once
#include "d3dcommon.h"
#include "DescriptorAllocator.h"

class GenerateMipmapBatch {
public:
	static bool Generate(ID3D12Resource* res,D3D12_RESOURCE_STATES initState,
		size_t width,size_t height,
		size_t mipnum,Descriptor* destHandles,
		Descriptor* srcHandle);
private:
	static bool initialize();
};