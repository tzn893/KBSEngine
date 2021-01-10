#pragma once
#include "PipelineStateObject.h"
#include "RootSignature.h"
#include "d3dcommon.h"
#include "graphic.h"

class ComputeCommand {
public:
	ComputeCommand(Graphic* graphic);
	ComputeCommand() { graphic = nullptr; valid = false; }

	void SetCPSOAndRootSignature(const wchar_t* cPso,const wchar_t* cRootSig);
	
	void BindConstantBuffer(size_t slot,D3D12_GPU_VIRTUAL_ADDRESS gpuAddr);
	void BindDescriptorHandle(size_t slot,D3D12_GPU_DESCRIPTOR_HANDLE gHandle);
	inline void BindDescriptorHeap(ID3D12DescriptorHeap* const * heaps, size_t heapNum) {
		graphic->BindDescriptorHeap(heaps, heapNum);
	}
	inline void BindDescriptorHeap(ID3D12DescriptorHeap* heaps) {
		graphic->BindDescriptorHeap(heaps);
	}

	inline void ResourceTrasition(ID3D12Resource* resource, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after) {
		graphic->ResourceTransition(resource, before, after);
	}
	inline void ResourceCopy(ID3D12Resource* Dest, ID3D12Resource* Source) {
		graphic->ResourceCopy(Dest, Source);
	}
	inline void ResourceCopy(ID3D12Resource* Dest, ID3D12Resource* Source, D3D12_RESOURCE_STATES destInitState,
		D3D12_RESOURCE_STATES sourceInitState, D3D12_RESOURCE_STATES destAfterState,
		D3D12_RESOURCE_STATES sourceAfterState) {
		graphic->ResourceCopy(Dest, Source, destInitState, sourceInitState, destAfterState, sourceAfterState);
	}

	void Dispatch(size_t x,size_t y,size_t z);

	static ComputeCommand Begin() {
		if (gGraphic.state == Graphic::BEGIN_COMMAND_RECORDING) {
			return ComputeCommand(&gGraphic);
		}
		return ComputeCommand();
	}

	void End();
private:
	Graphic* graphic;
	bool valid;
};