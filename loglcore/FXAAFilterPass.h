#pragma once
#include "PostProcessRenderPass.h"
#include "DescriptorAllocator.h"
#include "Texture.h"

class FXAAFilterPass : public PostProcessPass {
public:
	virtual size_t GetPriority() { return 5; }
	virtual void   PostProcess(Texture* RT, ComputeCommand* cc) override;

	virtual bool Initialize(PostProcessRenderPass* pPPRP) override;
	virtual void finalize() override;
private:
	const wchar_t* lumaPsoName = L"fxaa_luma";
	float filterThreshold = .5f;
};