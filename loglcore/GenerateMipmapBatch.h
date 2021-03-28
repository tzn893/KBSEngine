#pragma once
#include "d3dcommon.h"
#include "DescriptorAllocator.h"

class GenerateMipmapBatch {
public:
	static bool Generate(ID3D12Resource* res,D3D12_RESOURCE_STATES initState,
		size_t width,size_t height,
		size_t mipnum,Descriptor* destHandles,
		Descriptor* srcHandle);

	static bool GenerateIBLIrradience(ID3D12Resource* source,ID3D12Resource* target,
		Descriptor srcSrvHandle,
		D3D12_RESOURCE_STATES tarInitState);

	static bool PrefilterEnvironment(ID3D12Resource* source,ID3D12Resource* target,
		size_t mipnum,Descriptor srcSrvHandle,
		D3D12_RESOURCE_STATES tarInitState);

	static bool GenerateHDRCubeMap(ID3D12Resource* res,ID3D12Resource* target,
		D3D12_RESOURCE_STATES resState,D3D12_RESOURCE_STATES targetState);

private:
	//we will read the lut directly from disk rather than compute them in realtime
	static bool GenerateEnvLUT(ID3D12Resource* target, D3D12_RESOURCE_STATES tarInitState,
		DXGI_FORMAT targetFormat);

	static bool initialize();
};