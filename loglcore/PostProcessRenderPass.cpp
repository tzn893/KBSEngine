#include "PostProcessRenderPass.h"
#include "ComputeCommand.h"
#include "graphic.h"

bool PostProcessRenderPass::Initialize(UploadBatch* batch) {
	size_t winHeight = gGraphic.GetScreenHeight(), winWidth = gGraphic.GetScreenWidth();
	mTex = std::make_unique<Texture>(winWidth,winHeight,TEXTURE_FORMAT_RGBA,TEXTURE_FLAG_ALLOW_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_COPY_SOURCE);

	mHeap = std::make_unique<DescriptorHeap>(heapDefaultCapacity);
	mTex->CreateUnorderedAccessView(mHeap->Allocate());
	return true;
}

void PostProcessRenderPass::RegisterPostProcessPass(PostProcessPass* pass) {
	size_t priority = pass->GetPriority();
	if (auto res = PPPQueue.find(priority);res != PPPQueue.end()) {
		res->second.push_back(pass);
		return;
	}
	PPPQueue[priority] = std::vector<PostProcessPass*>{ pass };
}

void PostProcessRenderPass::PostProcess(ID3D12Resource* renderTarget) {
	if (PPPQueue.empty()) {
		return;
	}

	{
		ComputeCommand cc = ComputeCommand::Begin();
		cc.ResourceTrasition(renderTarget, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_SOURCE);
		cc.ResourceTrasition(mTex->GetResource(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COPY_DEST);
		cc.ResourceCopy(mTex->GetResource(), renderTarget);
		cc.ResourceTrasition(mTex->GetResource(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

		for (auto ppps : PPPQueue) {
			for (auto* ppp : ppps.second) {
				ppp->PostProcess(mTex.get(), &cc);
			}
		}

		cc.ResourceTrasition(mTex->GetResource(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);
		cc.ResourceTrasition(renderTarget, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COPY_DEST);
		cc.ResourceCopy(renderTarget, mTex->GetResource());
		cc.ResourceTrasition(renderTarget, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_RENDER_TARGET);

		cc.End();
	}
}

void PostProcessRenderPass::finalize() {
	for (auto ppps : PPPQueue) {
		for (auto* ppp : ppps.second) {
			ppp->finalize();
		}
	}
	PPPQueue.clear();
	mTex.release();
	mHeap.release();
}