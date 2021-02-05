#include "FXAAFilterPass.h"
#include "ComputeCommand.h"
#include "graphic.h"

bool FXAAFilterPass::Initialize(PostProcessRenderPass* pPPRP) {
	Game::RootSignature rootSig(3,1);
	rootSig[0].initAsDescriptorTable(0, 0, 1, D3D12_DESCRIPTOR_RANGE_TYPE_UAV);
	rootSig[1].initAsConstants(0, 0, 3);
	rootSig[2].initAsDescriptorTable(0, 0, 1, D3D12_DESCRIPTOR_RANGE_TYPE_SRV);
	rootSig.InitializeSampler(0, CD3DX12_STATIC_SAMPLER_DESC(0));

	if (!gGraphic.CreateRootSignature(lumaPsoName,&rootSig)) {
		OUTPUT_DEBUG_STRING("fail to create root signature for fxaa filter\n");
		return false;
	}

	ComputeShader* shader = gShaderManager.loadComputeShader(L"../shader/FXAAFilter.hlsl",
		"CalculateLuminance", lumaPsoName, lumaPsoName, nullptr);
	if (shader == nullptr) {
		OUTPUT_DEBUG_STRING("fail to create shader for fxaa filter\n");
		return false;
	}


	Game::ComputePSO pso = Game::ComputePSO::Default();
	if (!gGraphic.CreateComputePipelineStateObject(shader,&pso,lumaPsoName)) {
		OUTPUT_DEBUG_STRING("fail to create pso for fxaa filter\n");
		return false;
	}

	return true;
}

void FXAAFilterPass::PostProcess(Texture* RT, ComputeCommand* cc) {
	RT->StateTransition(D3D12_RESOURCE_STATE_COMMON);
	cc->SetCPSOAndRootSignature(lumaPsoName, lumaPsoName);
	cc->BindDescriptorHandle(0, RT->GetUnorderedAccessViewGPU());
	cc->BindDescriptorHandle(2, RT->GetShaderResourceViewGPU());

	UINT width = (UINT)gGraphic.GetScreenWidth(), 
		height = (UINT)gGraphic.GetScreenHeight();
	cc->Bind32bitConstant(1, width, 0);
	cc->Bind32bitConstant(1, height, 1);
	cc->Bind32bitConstant(1, filterThreshold, 2);

	UINT disx = round_up(width, 8) / 8, disy = round_up(height, 8) / 8;
	cc->Dispatch(disx, disy, 1);
}

void FXAAFilterPass::finalize() {
	//luma = nullptr;
}