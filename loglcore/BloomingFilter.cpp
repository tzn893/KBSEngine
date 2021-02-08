#include "BloomingFilter.h"
#include "graphic.h"
#include "ComputeCommand.h"

bool BloomingFilter::Initialize(PostProcessRenderPass* pPPRP) {
	Game::RootSignature clipRootSig(3,1);
	clipRootSig[0].initAsDescriptorTable(0, 0, 1, D3D12_DESCRIPTOR_RANGE_TYPE_SRV);
	clipRootSig[1].initAsDescriptorTable(0, 0, 1, D3D12_DESCRIPTOR_RANGE_TYPE_UAV);
	clipRootSig[2].initAsConstants(0, 0, 3);
	D3D12_STATIC_SAMPLER_DESC ssd = CD3DX12_STATIC_SAMPLER_DESC(0);
	ssd.Filter = D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
	clipRootSig.InitializeSampler(0, ssd);
	if (!gGraphic.CreateRootSignature(bloom_pso_name_clip,&clipRootSig)) {
		OUTPUT_DEBUG_STRING("fail to create root signature for bloom filter\n");
		return false;
	}

	ComputeShader* clipShader = gShaderManager.loadComputeShader(L"../shader/BloomingDownSample.hlsl",
		"DownSample", bloom_pso_name_clip, bloom_pso_name_clip,
		nullptr);
	if (clipShader == nullptr) {
		OUTPUT_DEBUG_STRING("fail to create compute shader for bloom filter\n");
		return false;
	}

	Game::ComputePSO clipPso = Game::ComputePSO::Default();
	if (!gGraphic.CreateComputePipelineStateObject(clipShader,&clipPso,bloom_pso_name_clip)) {
		OUTPUT_DEBUG_STRING("fail to create pso for bloom filter\n");
		return false;
	}

	Game::RootSignature blurRootSig(3, 0);
	blurRootSig[0].initAsDescriptorTable(0, 0, 1, D3D12_DESCRIPTOR_RANGE_TYPE_UAV);
	blurRootSig[1].initAsDescriptorTable(1, 0, 1, D3D12_DESCRIPTOR_RANGE_TYPE_UAV);
	blurRootSig[2].initAsConstants(0, 0, 2);

	if (!gGraphic.CreateRootSignature(bloom_pso_name_blur,&blurRootSig)) {
		OUTPUT_DEBUG_STRING("fail to create root signature for blur filter\n");
		return false;
	}

	ComputeShader* blurComputeShader = gShaderManager.loadComputeShader(L"../shader/BloomingBlur.hlsl",
		"Blur", bloom_pso_name_blur, bloom_pso_name_blur);
	if (blurComputeShader == nullptr) {
		OUTPUT_DEBUG_STRING("fail to create shader for blur filter\n");
		return false;
	}

	Game::ComputePSO blurPso = Game::ComputePSO::Default();
	if (!gGraphic.CreateComputePipelineStateObject(blurComputeShader,&blurPso)) {
		OUTPUT_DEBUG_STRING("fail to pso for blur filter\n");
		return false;
	}

	Game::RootSignature combRoot(3, 1);
	combRoot[0].initAsDescriptorTable(0, 0, 1, D3D12_DESCRIPTOR_RANGE_TYPE_SRV);
	combRoot[1].initAsDescriptorTable(0, 0, 1, D3D12_DESCRIPTOR_RANGE_TYPE_UAV);
	combRoot[2].initAsConstants(0, 0, 6);
	combRoot.InitializeSampler(0, ssd);
	if (!gGraphic.CreateRootSignature(bloom_pso_name_combine, &combRoot)) {
		OUTPUT_DEBUG_STRING("fail to create root signature for bloom filter\n");
		return false;
	}

	ComputeShader* combineShader = gShaderManager.loadComputeShader(L"../shader/BloomingCombine.hlsl",
		"Combine", bloom_pso_name_combine, bloom_pso_name_combine,
		nullptr);
	if (combineShader == nullptr) {
		OUTPUT_DEBUG_STRING("fail to create compute shader for bloom filter\n");
		return false;
	}

	Game::ComputePSO combinePso = Game::ComputePSO::Default();
	if (!gGraphic.CreateComputePipelineStateObject(combineShader, &combinePso, bloom_pso_name_combine)) {
		OUTPUT_DEBUG_STRING("fail to create pso for bloom filter\n");
		return false;
	}
	bloom_width = 1024, bloom_height = 1024;
	helper[0] = std::make_unique<Texture>(bloom_width, bloom_height,  TEXTURE_FORMAT_FLOAT4, TEXTURE_FLAG_ALLOW_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	helper[1] = std::make_unique<Texture>(bloom_width, bloom_height,  TEXTURE_FORMAT_FLOAT4, TEXTURE_FLAG_ALLOW_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	helper[0]->CreateShaderResourceView(pPPRP->AllocateDescriptor());
	helper[1]->CreateShaderResourceView(pPPRP->AllocateDescriptor());
	Descriptor d1 = pPPRP->AllocateDescriptor(4), d2 = pPPRP->AllocateDescriptor(4);
	for (size_t i = 0; i != 1;i++) {
		helper[0]->CreateUnorderedAccessView(d1, nullptr, i);
		helper[1]->CreateUnorderedAccessView(d2, nullptr, i);
		d1 = d1.Offset(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1),
		d2 = d2.Offset(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1);
	}

	weights = { 3.,.75,.5,.5 };

	return true;
}

void BloomingFilter::finalize() {
	helper[0] = nullptr, helper[1] = nullptr;
}


void BloomingFilter::PostProcess(Texture* RT, ComputeCommand* cc) {
	cc->SetCPSOAndRootSignature(bloom_pso_name_clip, bloom_pso_name_clip);
	RT->StateTransition(D3D12_RESOURCE_STATE_COMMON);
	cc->BindDescriptorHandle(0, RT->GetShaderResourceViewGPU());
	cc->BindDescriptorHandle(1, helper[0]->GetUnorderedAccessViewGPU(0));
	
	cc->Bind32bitConstant(2, threshold, 0);
	cc->Bind32bitConstant(2, (UINT)bloom_width, 1);
	cc->Bind32bitConstant(2, (UINT)bloom_height, 2);

	cc->Dispatch(bloom_width / 8, bloom_height / 8, 1);

	cc->SetCPSOAndRootSignature(bloom_pso_name_blur, bloom_pso_name_blur);
	size_t input = 0, output = 1;

	for (size_t i = 0; i != 4; i++) {
		cc->BindDescriptorHandle(0, helper[output]->GetUnorderedAccessViewGPU(0));
		cc->BindDescriptorHandle(1, helper[input]->GetUnorderedAccessViewGPU(0));


		UINT width = bloom_width, height = bloom_height;
		cc->Bind32bitConstant(2, width, 0);
		cc->Bind32bitConstant(2, height, 1);
		cc->Dispatch(width / 8, height / 8, 1);
		std::swap(output, input);
	}

	helper[input]->StateTransition(D3D12_RESOURCE_STATE_COMMON);

	cc->SetCPSOAndRootSignature(bloom_pso_name_combine, bloom_pso_name_combine);
	RT->StateTransition(D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	cc->Bind32bitConstant(2, weights, 2);
	cc->Bind32bitConstant(2, (UINT)RT->GetWidth(), 0);
	cc->Bind32bitConstant(2, (UINT)RT->GetHeight(), 1);

	cc->BindDescriptorHandle(0,helper[input]->GetShaderResourceViewGPU());
	cc->BindDescriptorHandle(1, RT->GetUnorderedAccessViewGPU());

	helper[input]->StateTransition(D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	size_t disx = round_up(RT->GetWidth(), 8) / 8,
		disy = round_up(RT->GetHeight(), 8) / 8;
	cc->Dispatch(disx, disy, 1);
}