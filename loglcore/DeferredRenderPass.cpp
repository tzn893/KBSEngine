#include "DeferredRenderPass.h"
#include "graphic.h"
#include "TextureManager.h"
#include "LightManager.h"

bool DeferredRenderPass::Initialize(UploadBatch* batch) {
	Game::RootSignature rootSig(6,1);
	rootSig[0].initAsConstantBuffer(0, 0);
	rootSig[1].initAsConstantBuffer(1, 0);
	rootSig[2].initAsDescriptorTable(0, 0, 1, D3D12_DESCRIPTOR_RANGE_TYPE_SRV);
	rootSig[3].initAsDescriptorTable(1, 0, 1, D3D12_DESCRIPTOR_RANGE_TYPE_SRV);
	rootSig[4].initAsDescriptorTable(2, 0, 1, D3D12_DESCRIPTOR_RANGE_TYPE_SRV);
	rootSig[5].initAsDescriptorTable(3, 0, 1, D3D12_DESCRIPTOR_RANGE_TYPE_SRV);
	rootSig.InitializeSampler(0, CD3DX12_STATIC_SAMPLER_DESC(0));

	if (!gGraphic.CreateRootSignature(defPreproc,&rootSig)) {
		OUTPUT_DEBUG_STRING("fail to create root signature for deferred render pass\n");
		return false;
	}
	std::vector<D3D12_INPUT_ELEMENT_DESC> layout = {
		{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
		{"NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,12,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
		{"TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,24,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
		{"TANGENT",0,DXGI_FORMAT_R32G32B32_FLOAT,0,32,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
	};

	Shader* shader = gShaderManager.loadShader(L"../shader/DeferredRPGBuffer.hlsl",
		"VS", "PS", defPreproc, layout, defPreproc, nullptr);
	if (shader == nullptr) {
		OUTPUT_DEBUG_STRING("fail to create shader for deferred render pass\n");
		return false;
	}

	Game::GraphicPSO GBufferPSO;
	GBufferPSO.LazyBlendDepthRasterizeDefault();
	GBufferPSO.SetSampleMask(UINT_MAX);
	GBufferPSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
	GBufferPSO.SetFlag(D3D12_PIPELINE_STATE_FLAG_NONE);
	DXGI_FORMAT rtvFormats[GBufferNum] = { DXGI_FORMAT_R32G32B32A32_FLOAT ,DXGI_FORMAT_R32G32B32A32_FLOAT ,DXGI_FORMAT_R32G32B32A32_FLOAT };
	GBufferPSO.SetRenderTargetFormat(GBufferNum, rtvFormats);
	GBufferPSO.SetDepthStencilViewFomat(DXGI_FORMAT_D24_UNORM_S8_UINT);

	if (!gGraphic.CreatePipelineStateObject(shader,&GBufferPSO,defPreproc)) {
		OUTPUT_DEBUG_STRING("fail to create shader for deferred render pass\n");
		return false;
	}

	descriptorHeap = std::make_unique<DescriptorHeap>(defaultDescriptorHeapSize);
	descriptorHeap->ClearUploadedDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	Descriptor desc = descriptorHeap->Allocate(3, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	D3D12_CLEAR_VALUE cv;
	cv.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	memset(cv.Color, 0, sizeof(cv.Color));

	for (size_t i = 0; i != GBufferNum;i++) {
		GBuffer[i] = std::make_unique<Texture>(gGraphic.GetScreenWidth(), gGraphic.GetScreenHeight(),
			TEXTURE_FORMAT_FLOAT4, TEXTURE_FLAG_ALLOW_RENDER_TARGET, D3D12_RESOURCE_STATE_COMMON, &cv);
		GBuffer[i]->CreateRenderTargetView(descriptorHeap->Allocate(1, D3D12_DESCRIPTOR_HEAP_TYPE_RTV));
		GBuffer[i]->CreateShaderResourceView(desc.Offset(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, i));
	}
	

	allocatedConstantBuffers = 0;
	ID3D12Device* device = gGraphic.GetDevice();
	objConstants = std::make_unique<ConstantBuffer<ObjectPass>>(device,defaultConstantBufferSize);
	
	Game::Vector4 rect[] = {
		{-1.,-1.,0.,1.},
		{ 1.,-1.,1.,1.},
		{ 1., 1.,1.,0.},
		{-1., 1.,0.,0.},
		{-1.,-1.,0.,1.},
		{ 1., 1.,1.,0.}
	};

	mImageVert = std::make_unique<StaticMesh<Game::Vector4>>(device, _countof(rect), rect, batch);

	Game::RootSignature rootSigShading(4, 2);
	rootSigShading[0].initAsConstantBuffer(0, 0);
	rootSigShading[1].initAsConstantBuffer(1, 0);
	rootSigShading[2].initAsDescriptorTable(0, 0, 3, D3D12_DESCRIPTOR_RANGE_TYPE_SRV);
	rootSigShading[3].initAsDescriptorTable(3, 0, 1, D3D12_DESCRIPTOR_RANGE_TYPE_SRV);
	D3D12_SAMPLER_DESC sd;
	sd.AddressV = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
	rootSigShading.InitializeSampler(0, CD3DX12_STATIC_SAMPLER_DESC(0));
	rootSigShading.InitializeSampler(1, CD3DX12_STATIC_SAMPLER_DESC(1));

	if (!gGraphic.CreateRootSignature(defShading,&rootSigShading)) {
		OUTPUT_DEBUG_STRING("fail to create root signature for deferred render pass\n");
		return false;
	}

	std::vector<D3D12_INPUT_ELEMENT_DESC> layoutShading = {
		{"POSITION",0,DXGI_FORMAT_R32G32_FLOAT,0,0,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
		{"TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,8,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0}
	};
	D3D_SHADER_MACRO macros[] = {
		{"MAX_LIGHT_STRUCT_NUM",getShaderMaxLightStructNumStr},
		{nullptr,nullptr}
	};

	shader = gShaderManager.loadShader(L"../shader/DeferredRPShading.hlsl", "VS", "PS",
		defShading, layoutShading, defShading, macros);

	Game::GraphicPSO GBufferShadingPSO;

	GBufferShadingPSO.LazyBlendDepthRasterizeDefault();
	CD3DX12_DEPTH_STENCIL_DESC ds(D3D12_DEFAULT);
	ds.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	ds.DepthFunc      = D3D12_COMPARISON_FUNC_ALWAYS;
	GBufferShadingPSO.SetDepthStencilState(ds);
	
	CD3DX12_RASTERIZER_DESC rd(D3D12_DEFAULT);
	rd.CullMode = D3D12_CULL_MODE_NONE;
	GBufferShadingPSO.SetRasterizerState(rd);
	
	GBufferShadingPSO.SetSampleMask(UINT_MAX);
	GBufferShadingPSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
	GBufferShadingPSO.SetFlag(D3D12_PIPELINE_STATE_FLAG_NONE);
	GBufferShadingPSO.SetRenderTargetFormat(gGraphic.GetRenderTargetFormat());
	GBufferShadingPSO.SetDepthStencilViewFomat(gGraphic.GetBackBufferDepthFormat());

	if (!gGraphic.CreatePipelineStateObject(shader,&GBufferShadingPSO,defShading)) {
		OUTPUT_DEBUG_STRING("fail to create pso for deferred render pass");
		return false;
	}


	return true;
}

void DeferredRenderPass::Render(Graphic* graphic, RENDER_PASS_LAYER layer) {
	if (layer != RENDER_PASS_LAYER_OPAQUE) return;
	if (objQueue.empty()) return;
	graphic->BindPSOAndRootSignature(defPreproc, defPreproc);
	
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[GBufferNum] = { GBuffer[0]->GetRenderTargetViewCPU(),
		GBuffer[1]->GetRenderTargetViewCPU(),GBuffer[2]->GetRenderTargetViewCPU() };

	TransitionBatch tbatch = TransitionBatch::Begin();
	D3D12_RESOURCE_STATES resLastState;
	for (size_t i = 0; i != GBufferNum;i++) {
		resLastState = GBuffer[i]->StateTransition(D3D12_RESOURCE_STATE_RENDER_TARGET, &tbatch);
	}
	tbatch.End();

	Game::Vector4 ClearValue = Game::Vector4();
	graphic->BindRenderTarget(rtvHandles, { 0 }, GBufferNum, true,
		ClearValue.raw, nullptr, nullptr);
	graphic->BindDescriptorHeap(descriptorHeap->GetHeap());
	graphic->BindMainCameraPass(1);
	

	for (auto& ele : objQueue) {
		graphic->BindConstantBuffer(objConstants->GetADDR(ele.objectID), 0);
		graphic->BindDescriptorHandle(ele.normalMap, 2);
		graphic->BindDescriptorHandle(ele.diffuseMap, 3);
		graphic->BindDescriptorHandle(ele.roughnessMap, 4);
		graphic->BindDescriptorHandle(ele.metallicMap, 5);
		if (ele.ibv == nullptr) {
			graphic->Draw(ele.vbv, ele.start, ele.num);
		}
		else{
			graphic->Draw(ele.vbv, ele.ibv, ele.start, ele.num);
		}
		
	}
	tbatch = TransitionBatch::Begin();
	for (size_t i = 0; i != GBufferNum; i++) {
		GBuffer[i]->StateTransition(resLastState, &tbatch);
	}
	tbatch.End();

	graphic->BindCurrentBackBufferAsRenderTarget();

	SkyboxRenderPass* sbrp = gGraphic.GetRenderPass<SkyboxRenderPass>();
	Texture* irrMap = sbrp->GetIrradianceMap();
	D3D12_GPU_DESCRIPTOR_HANDLE irrDescHandle = descriptorHeap->UploadDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
		irrMap->GetShaderResourceViewCPU()).gpuHandle;

	graphic->BindPSOAndRootSignature(defShading, defShading);
	gLightManager.BindLightPass2ConstantBuffer(0);
	graphic->BindMainCameraPass(1);
	graphic->BindDescriptorHandle(GBuffer[0]->GetShaderResourceViewGPU(),2);
	graphic->BindDescriptorHandle(irrDescHandle, 3);
	graphic->Draw(mImageVert->GetVBV(), 0, mImageVert->GetVertexNum());

	descriptorHeap->ClearUploadedDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	objQueue.clear();
}

void DeferredRenderPass::finalize() {

}

DeferredRenderPassID  DeferredRenderPass::AllocateObjectPass() {
	if (!availableConstantBuffers.empty()) {
		DeferredRenderPassID rv = availableConstantBuffers[availableConstantBuffers.size() - 1];
		availableConstantBuffers.pop_back();
		return rv;
	}
	else if (allocatedConstantBuffers < defaultConstantBufferSize) {
		DeferredRenderPassID id = allocatedConstantBuffers++;
		return id;
	}
	return -1;
}


void DeferredRenderPass::DeallocateObjectPass(DeferredRenderPassID& objId) {
	if (defaultConstantBufferSize > objId) {
		availableConstantBuffers.push_back(objId);
		objId = -1;
	}
}

ObjectPass* DeferredRenderPass::GetObjectPass(DeferredRenderPassID id) {
	if (id >= defaultConstantBufferSize) return nullptr;
	return objConstants->GetBufferPtr(id);
}

void DeferredRenderPass::DrawObject(D3D12_VERTEX_BUFFER_VIEW* vbv, D3D12_INDEX_BUFFER_VIEW* ibv,
	size_t start, size_t num, DeferredRenderPassID id, DeferredRenderPassTexture* tex) {
	if (id < 0 || id >= defaultConstantBufferSize) {return;}
	
	Texture* white = gTextureManager.getWhiteTexture();
	D3D12_GPU_DESCRIPTOR_HANDLE whiteHandle{0};
	if (tex->diffuse == nullptr  || tex->metallic == nullptr
		 || tex->roughness == nullptr) {
		whiteHandle = descriptorHeap->UploadDescriptors(
			D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 
			white->GetShaderResourceViewCPU()
		).gpuHandle;
	}

	D3D12_GPU_DESCRIPTOR_HANDLE normalHandle{0};
	if (tex->normal == nullptr) {
		normalHandle = descriptorHeap->UploadDescriptors(
			D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
			gTextureManager.getNormalMapDefaultTexture()->GetShaderResourceViewCPU()
		).gpuHandle;
	}
	
	ObjectElement oe;
	oe.ibv = ibv;
	oe.vbv = vbv;
	oe.start = start;
	oe.num = num;
	oe.objectID = id;

	auto uploadTexture2CurrentHeap = [&](Texture* tex,D3D12_GPU_DESCRIPTOR_HANDLE spare) {
		if (tex == nullptr) return spare;
		if (tex->GetShaderResourceViewCPU().ptr == 0) {
			tex->CreateShaderResourceView(gDescriptorAllocator.AllocateDescriptor());
		}
		return descriptorHeap->UploadDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, tex->GetShaderResourceViewCPU()).gpuHandle;
	};

	oe.diffuseMap = uploadTexture2CurrentHeap(tex->diffuse,whiteHandle);
	oe.normalMap = uploadTexture2CurrentHeap(tex->normal,normalHandle);
	oe.roughnessMap = uploadTexture2CurrentHeap(tex->roughness,whiteHandle);
	oe.metallicMap = uploadTexture2CurrentHeap(tex->metallic,whiteHandle);

	objQueue.push_back(oe);
}