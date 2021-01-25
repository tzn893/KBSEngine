#include "PipelineStateObject.h"
#include "d3d12x.h"

using namespace Game;

bool GraphicPSO::Create(ID3D12Device* device) {
	if (!m_PSODesc.pRootSignature) {
		printf("GraphicPSO::Create : rootsignature can't be a null when creating\n");
		return false;
	}
	/*if (m_InputLayouts.empty()) {
		printf("GraphicPSO::Create : can't create pso with empty input layout");
		return false;
	}*/

	D3D12_INPUT_LAYOUT_DESC layout;
	layout.NumElements = m_InputLayouts.size();
	layout.pInputElementDescs = m_InputLayouts.empty() ? nullptr : m_InputLayouts.data();

	m_PSODesc.InputLayout = layout;
	HRESULT hr = device->CreateGraphicsPipelineState(
		&m_PSODesc, IID_PPV_ARGS(&m_Pso)
	);
	if (FAILED(hr)) {
		printf("GraphicPSO::Create :: fail to create pipeline state object");
		return false;
	}

	return true;
}

void GraphicPSO::SetRootSignature(RootSignature* rootSig) {
	if (!rootSig->RootIsCreated()) {
		printf("GraphicPSO::SetRootSignature : set rootsignature before it has been created ");
		return;
	}
	m_PSODesc.pRootSignature = rootSig->GetRootSignature();
}


void GraphicPSO::LazyBlendDepthRasterizeDefault() {
	m_PSODesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	m_PSODesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	m_PSODesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
}

void GraphicPSO::SetRenderTargetFormat(UINT num,const DXGI_FORMAT* formats) {
	//ASSERT_WARNING(num == 0 || formats == nullptr ,"GraphicPSO::SetRenderTargetFormat the num of render target can't be 0");
	if (num == 0 || formats == nullptr) {
		printf("GraphicPSO::SetRenderTargetFormat the num of render target can't be 0");
		return;
	}
	for (int i = 0; i != num; i++) {
		m_PSODesc.RTVFormats[i] = formats[i];
	}
	for (int i = num; i != _countof(m_PSODesc.RTVFormats); i++) {
		m_PSODesc.RTVFormats[i] = DXGI_FORMAT_UNKNOWN;
	}
	m_PSODesc.NumRenderTargets = num;
}

ID3D12PipelineState* GraphicPSO::GetPSO() {
	//ASSERT_WARNING(m_Pso.Get() == nullptr,"you must create the pso before you can get it");
	if (m_Pso.Get() == nullptr) 
		printf("you must create the pso before you can get it");
	return m_Pso.Get();
}