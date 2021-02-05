#pragma once
#include "PostProcessRenderPass.h"
#include "Texture.h"


class BloomingFilter : public PostProcessPass {
public:
	virtual size_t GetPriority() { return 10; }
	virtual void   PostProcess(Texture* RT, ComputeCommand* cc) override;

	virtual bool Initialize(PostProcessRenderPass* pPPRP) override;
	virtual void finalize() override;
private:
	std::unique_ptr<Texture> helper[2];
	const wchar_t* bloom_pso_name_clip = L"bloom_filter_clip";
	const wchar_t* bloom_pso_name_blur = L"bloom_filter_blur";
	const wchar_t* bloom_pso_name_combine = L"bloom_fileter_combine";

	struct BloomWeight {
		float weight[4];
	} weights;

	float threshold = 3.;
	UINT width;
	UINT bloom_width, bloom_height;
};