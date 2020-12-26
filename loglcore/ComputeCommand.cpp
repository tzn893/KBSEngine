#include "ComputeCommand.h"
#include "graphic.h"

ComputeCommand::ComputeCommand(Graphic* graphic) {
	valid = true;
	this->graphic = graphic;
}

void ComputeCommand::SetCPSOAndRootSignature(const wchar_t* cPso, const wchar_t* cRootSig) {
	if (!valid || graphic->state != Graphic::BEGIN_COMMAND_RECORDING) {
		return;
	}
	ID3D12PipelineState* pso = nullptr;
	ID3D12RootSignature* rootSig = nullptr;
	if (auto i = graphic->mPsos.find(cPso);i != graphic->mPsos.end()) {
		pso = i->second.Get();
	}
	if (auto i = graphic->mRootSignatures.find(cRootSig);i != graphic->mRootSignatures.end()) {
		rootSig = i->second.Get();
	}

	if (pso != nullptr && rootSig != nullptr) {
		graphic->mDrawCmdList->SetPipelineState(pso);
		graphic->mDrawCmdList->SetComputeRootSignature(rootSig);
	}
}

inline void ComputeCommand::BindConstantBuffer(size_t slot, D3D12_GPU_VIRTUAL_ADDRESS gpuAddr) {
	if (!valid || graphic->state != Graphic::BEGIN_COMMAND_RECORDING) {
		return;
	}
	graphic->mDrawCmdList->SetComputeRootConstantBufferView(slot, gpuAddr);
}
inline void ComputeCommand::BindDescriptorHandle(size_t slot, D3D12_GPU_DESCRIPTOR_HANDLE gHandle) {
	if (!valid || graphic->state != Graphic::BEGIN_COMMAND_RECORDING) {
		return;
	}
	graphic->mDrawCmdList->SetComputeRootDescriptorTable(slot, gHandle);
}

inline void ComputeCommand::BindDescriptorHeap(ID3D12DescriptorHeap* const * heaps, size_t heapNum) {
	graphic->BindDescriptorHeap(heaps, heapNum);
}

inline void ComputeCommand::ResourceTrasition(ID3D12Resource* resource, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after) {
	graphic->ResourceTransition(resource, before, after);
}
inline void ComputeCommand::ResourceCopy(ID3D12Resource* Dest, ID3D12Resource* Source) {
	graphic->ResourceCopy(Dest, Source);
}
inline void ComputeCommand::ResourceCopy(ID3D12Resource* Dest, ID3D12Resource* Source, D3D12_RESOURCE_STATES destInitState,
	D3D12_RESOURCE_STATES sourceInitState, D3D12_RESOURCE_STATES destAfterState,
	D3D12_RESOURCE_STATES sourceAfterState) {
	graphic->ResourceCopy(Dest,Source,destInitState,sourceInitState,destAfterState,sourceAfterState);
}

void ComputeCommand::Dispatch(size_t x, size_t y, size_t z) {
	if (!valid || graphic->state != Graphic::BEGIN_COMMAND_RECORDING) {
		return;
	}
	graphic->mDrawCmdList->Dispatch(x, y, z);
}

void ComputeCommand::End() {
	graphic = nullptr;
	valid = false;
}

