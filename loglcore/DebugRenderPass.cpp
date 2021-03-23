#include "DebugRenderPass.h"

#include "RootSignature.h"
#include "PipelineStateObject.h"
#include "graphic.h"

bool   DebugRenderPass::Initialize(UploadBatch* batch) {

	Game::RootSignature rootSig(2,1);
	rootSig[0].initAsDescriptorTable(0, 0, 1, D3D12_DESCRIPTOR_RANGE_TYPE_SRV);
	rootSig[1].initAsConstantBuffer(0, 0);

	rootSig.InitializeSampler(0, CD3DX12_STATIC_SAMPLER_DESC(0));
	if (!gGraphic.CreateRootSignature(L"debugDD",&rootSig)) {
		OUTPUT_DEBUG_STRING("fail to create root signature for debug render pass\n");
		return false;
	}

	std::vector<D3D12_INPUT_ELEMENT_DESC> lDesc = {
		{"POSITION",0,DXGI_FORMAT_R32G32_FLOAT,0,0,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
		{"TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,8,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0}
	};

	shader = gShaderManager.loadShader(L"../shader/Legacy/DebugDirectDraw.hlsl", "VS", "PS",
		L"debugDD", lDesc, L"debugDD");
	if (shader == nullptr) {
		OUTPUT_DEBUG_STRING("fail to create shader for debug render pass\n");
		return false;
	}

	Game::GraphicPSORP mPso;
	mPso.LazyBlendDepthRasterizeDefault();
	CD3DX12_RASTERIZER_DESC rd(D3D12_DEFAULT);
	rd.CullMode = D3D12_CULL_MODE_NONE;
	mPso.SetRasterizerState(rd);
	CD3DX12_DEPTH_STENCIL_DESC ds(D3D12_DEFAULT);
	ds.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	mPso.SetDepthStencilState(ds);
	mPso.SetNodeMask(0);
	mPso.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
	mPso.SetFlag(D3D12_PIPELINE_STATE_FLAG_NONE);

	if (!gGraphic.CreatePipelineStateObjectRP(shader, &mPso)) {
		OUTPUT_DEBUG_STRING("fail to create pso for debug pass\n");
		return false;
	}

	Game::Vector4 rect[] = {
		{-1.,-1.,0.,0.},
		{ 1.,-1.,1.,0.},
		{ 1., 1.,1.,1.},
		{-1., 1.,0.,1.},
		{-1.,-1.,0.,0.},
		{ 1., 1.,1.,1.}
	};

	ddEnabled = false;
	ID3D12Device* mDevice = gGraphic.GetDevice();
	mesh = std::make_unique<StaticMesh<Game::Vector4>>(
		mDevice, _countof(rect), rect, batch
		);

	viewPort = std::make_unique<ConstantBuffer<DebugViewPort>>(mDevice);
	viewPort->GetBufferPtr()->downLeft = Game::Vector2(0., 0.);
	viewPort->GetBufferPtr()->upRight = Game::Vector2(1., 1.);
	
	mHeap = std::make_unique<DescriptorHeap>(1);
	return true;
}

void   DebugRenderPass::Render(Graphic* graphic, RENDER_PASS_LAYER layer) {
	if (layer != RENDER_PASS_LAYER_AFTER_ALL || !ddEnabled) return;
	
	graphic->BindShader(shader);
	graphic->BindConstantBuffer(viewPort->GetADDR(), 1);
	if (targetHeap == nullptr) {
		ID3D12DescriptorHeap* heaps[] = { mHeap->GetHeap() };
		graphic->BindDescriptorHeap(heaps, _countof(heaps));
		graphic->BindDescriptorHandle(mHeap->GetHeap()->GetGPUDescriptorHandleForHeapStart(), 0);
	}
	else {
		ID3D12DescriptorHeap* heaps[] = { targetHeap };
		graphic->BindDescriptorHeap(heaps, _countof(heaps));
		graphic->BindDescriptorHandle(targetTexture->GetShaderResourceViewGPU(), 0);
	}

	graphic->Draw(mesh->GetVBV(), 0, mesh->GetVertexNum());

	ddEnabled = false;
	targetHeap = nullptr;
}

void   DebugRenderPass::finalize() {
	mHeap.release();
	mesh.release();
	
}

void   DebugRenderPass::DirectDraw(Texture* texture, ID3D12DescriptorHeap* heap) {
	ddEnabled = true;
	if (heap != nullptr) {
		targetTexture = texture;
		targetHeap = heap;
	}
	else {
		mHeap->ClearUploadedDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		mHeap->UploadDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, texture->GetShaderResourceViewCPU());
	}
}

void   DebugRenderPass::SetViewPort(Game::Vector2 downLeft, Game::Vector2 upRight) {
	viewPort->GetBufferPtr()->downLeft = downLeft;
	viewPort->GetBufferPtr()->upRight = upRight;
}