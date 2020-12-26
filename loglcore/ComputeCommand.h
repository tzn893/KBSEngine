#pragma once
#include "PipelineStateObject.h"
#include "RootSignature.h"


class Graphic;

class ComputeCommand {
public:
	ComputeCommand(Graphic* graphic);

	void SetCPSOAndRootSignature(const wchar_t* cPso,const wchar_t* cRootSig);
	
	void BindConstantBuffer(size_t slot,D3D12_GPU_VIRTUAL_ADDRESS gpuAddr);
	void BindDescriptorHandle(size_t slot,D3D12_GPU_DESCRIPTOR_HANDLE gHandle);
	void BindDescriptorHeap(ID3D12DescriptorHeap* const * heaps,size_t heapNum);

	void ResourceTrasition(ID3D12Resource* resource, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after);
	void ResourceCopy(ID3D12Resource* Dest, ID3D12Resource* Source);
	void ResourceCopy(ID3D12Resource* Dest, ID3D12Resource* Source, D3D12_RESOURCE_STATES destInitState,
		D3D12_RESOURCE_STATES sourceInitState, D3D12_RESOURCE_STATES destAfterState,
		D3D12_RESOURCE_STATES sourceAfterState);

	void Dispatch(size_t x,size_t y,size_t z);

	void End();
private:
	Graphic* graphic;
	bool valid;
};