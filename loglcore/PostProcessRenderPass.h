#pragma once
#include "RenderPass.h"
#include <map>
#include "Texture.h"

#include "DescriptorAllocator.h"

class PostProcessRenderPass;
class ComputeCommand;

class PostProcessPass {
public:
	virtual size_t GetPriority() = 0;
	virtual void   PostProcess(Texture* RT,ComputeCommand* cc) = 0;

	virtual bool Initialize(PostProcessRenderPass* pPPRP) = 0;
	virtual void finalize() = 0;
private:
	PostProcessRenderPass* pPPRP;
};

class PostProcessRenderPass : public RenderPass{
public:
	virtual size_t GetPriority() override { return 201; }

	virtual void   PreProcess() {}
	virtual bool   Initialize(UploadBatch* batch) override ;
	virtual void   Render(Graphic* graphic, RENDER_PASS_LAYER layer) {}

	virtual void   PostProcess(ID3D12Resource* renderTarget) override;

	virtual void   finalize() override;

	void RegisterPostProcessPass(PostProcessPass* pass);

	Descriptor AllocateDescriptor(size_t num = 1);
private:
	std::unique_ptr<Texture> mTex;
	std::unique_ptr<DescriptorHeap> mHeap;

	std::map<size_t, std::vector<PostProcessPass*>> PPPQueue;
	static constexpr size_t heapDefaultCapacity = 64;
};