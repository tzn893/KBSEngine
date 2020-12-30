#include "LightManager.h"
#include "graphic.h"


bool   ShadowRenderPass::Initialize(UploadBatch* batch) {
	ID3D12Device* mDevice = gGraphic.GetDevice();

	Game::RootSignature shadowRootSig(2, 0);
	shadowRootSig[0].initAsConstantBuffer(0, 0);
	shadowRootSig[1].initAsConstantBuffer(1, 0);
	if (!gGraphic.CreateRootSignature(shadowRootSigName, &shadowRootSig)) {
		OUTPUT_DEBUG_STRING("fail to create shadow pass root signature for shadow pass");
		return false;
	}

	std::vector<D3D12_INPUT_ELEMENT_DESC> layout = {
		{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
		{"NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,12,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
		{"TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,24,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
		{"TANGENT",0,DXGI_FORMAT_R32G32B32_FLOAT,0,32,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
	};

	Shader* shadowPassShader = gShaderManager.loadShader(
		L"../shader/ShadowMap.hlsl", "VS", "PS",
		shadowRootSigName.c_str(), layout, L"shadowPass");
	if (shadowPassShader == nullptr) {
		OUTPUT_DEBUG_STRING("fail to create shader for shadow pass\n");
		return false;
	}

	Game::GraphicPSO ShadowPSO;
	ShadowPSO.LazyBlendDepthRasterizeDefault();
	CD3DX12_RASTERIZER_DESC rd(D3D12_DEFAULT);
	rd.CullMode = D3D12_CULL_MODE_BACK;
	ShadowPSO.SetRasterizerState(rd);
	ShadowPSO.SetSampleMask(UINT_MAX);
	ShadowPSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
	ShadowPSO.SetFlag(D3D12_PIPELINE_STATE_FLAG_NONE);
	ShadowPSO.SetRenderTargetFormat(DXGI_FORMAT_R8G8B8A8_UNORM);
	ShadowPSO.SetDepthStencilViewFomat(DXGI_FORMAT_D24_UNORM_S8_UINT);

	if (!gGraphic.CreatePipelineStateObject(shadowPassShader, &ShadowPSO, shadowPsoName.c_str())) {
		OUTPUT_DEBUG_STRING("fail to create pipeline state object for shadow pipeline\n");
		return false;
	}

	mDepthLightView = std::make_unique<ConstantBuffer<ShadowLightPass>>(mDevice);

	D3D12_CLEAR_VALUE rtClear;
	rtClear.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	rtClear.Color[0] = 1., rtClear.Color[1] = 1., rtClear.Color[2] = 1., rtClear.Color[3] = 1.;

	D3D12_CLEAR_VALUE dsClear;
	dsClear.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsClear.DepthStencil.Depth = 1.f;
	dsClear.DepthStencil.Stencil = 0.f;

	mDepthRTVTex = std::make_unique<Texture>(depthWidth, depthHeight,
		TEXTURE_FORMAT_RGBA, TEXTURE_FLAG_ALLOW_RENDER_TARGET,
		D3D12_RESOURCE_STATE_COMMON, &rtClear);
	mDepthDSVTex = std::make_unique<Texture>(depthWidth, depthHeight,
		TEXTURE_FORMAT_DEPTH_STENCIL, TEXTURE_FLAG_ALLOW_DEPTH_STENCIL,
		D3D12_RESOURCE_STATE_DEPTH_WRITE, &dsClear);

	mDepthRect.bottom = depthHeight;
	mDepthRect.top = 0;
	mDepthRect.right = depthWidth;
	mDepthRect.left = 0;

	mDepthView.Width = depthWidth;
	mDepthView.Height = depthHeight;
	mDepthView.TopLeftX = 0;
	mDepthView.TopLeftY = 0;
	mDepthView.MinDepth = 0.f;
	mDepthView.MaxDepth = 1.f;

	mHeap = std::make_unique<DescriptorHeap>(1);

	mDepthRTVTex->CreateRenderTargetView(mHeap->Allocate(1, D3D12_DESCRIPTOR_HEAP_TYPE_RTV));
	mDepthRTVTex->CreateShaderResourceView(gDescriptorAllocator.AllocateDescriptor());
	mDepthDSVTex->CreateDepthStencilView(mHeap->Allocate(1, D3D12_DESCRIPTOR_HEAP_TYPE_DSV));

	orthoMat = Game::MatrixOrtho(-30., 30., -30., 30., -100., 100.);
	UpdateShadowLightView();

	return true;
}
void   ShadowRenderPass::Render(Graphic* graphic, RENDER_PASS_LAYER layer) {
	if (layer != RENDER_PASS_LAYER_BEFORE_ALL)  return;
	UpdateShadowLightView();
	float clv[4] = { 1.,1.,1.,1. };
	if (objQueue.empty()) return;
	graphic->ResourceTransition(mDepthRTVTex->GetResource(),
		D3D12_RESOURCE_STATE_COMMON,
		D3D12_RESOURCE_STATE_RENDER_TARGET);

	graphic->BindRenderTarget(
		&mDepthRTVTex->GetRenderTargetViewCPU(),
		mDepthDSVTex->GetDepthStencilViewCPU(),
		1, true, clv, &mDepthView, &mDepthRect
	);

	graphic->BindPSOAndRootSignature(shadowPsoName.c_str(), shadowRootSigName.c_str());
	graphic->BindConstantBuffer(mDepthLightView->GetADDR(), 1);

	for (auto& ele : objQueue) {
		graphic->BindConstantBuffer(ele.cBuffer, 0);
		if (ele.ibv != nullptr) {
			graphic->Draw(ele.vbv, ele.ibv, ele.start, ele.num);
		}
		else {
			graphic->Draw(ele.vbv, ele.start, ele.num);
		}
	}

	graphic->ResourceTransition(mDepthRTVTex->GetResource(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_COMMON);

	graphic->BindCurrentBackBufferAsRenderTarget();

	objQueue.clear();
}

void   ShadowRenderPass::finalize() {
	objQueue.clear();
	mHeap.release();
	mDepthDSVTex.release();
	mDepthRTVTex.release();
	mDepthLightView.release();
}

void   ShadowRenderPass::RenderShadowMap(D3D12_VERTEX_BUFFER_VIEW* vbv, D3D12_INDEX_BUFFER_VIEW* ibv,
	size_t start, size_t num, D3D12_GPU_VIRTUAL_ADDRESS cBuffer) {
	objQueue.push_back({ vbv,ibv,start,num,cBuffer});
}
void   ShadowRenderPass::RenderShadowMap(D3D12_VERTEX_BUFFER_VIEW* vbv,
	size_t start,size_t num,D3D12_GPU_VIRTUAL_ADDRESS cBuffer) {
	objQueue.push_back({ vbv,nullptr,start,num,cBuffer });
}
Texture* ShadowRenderPass::GetShadowMap() {
	return mDepthRTVTex.get();
}

void   ShadowRenderPass::UpdateShadowLightView() {
	Camera* mainCamera = gGraphic.GetMainCamera();
	Game::Vector3 Position = mainCamera->getPosition();
	Game::Vector3 lightDir = gLightManager.GetMainLightData().direction;
	Game::Vector3 lightPosition = Position - lightDir * shadowDistance;

	mDepthLightView->GetBufferPtr()->lightView = Game::mul(
		orthoMat,
		Game::MatrixLookAt(lightPosition, Position, Game::Vector3(0., 1., 0.))
	).T();
}

void   LightManager::BindLightPass2ConstantBuffer(size_t slot) {
	gGraphic.BindConstantBuffer(mLightPass->GetADDR(), slot);
}

bool   LightManager::EnableShadowRenderPass() {
	if (shadowEnabled) return true;
	mShadowRenderPass = std::make_unique<ShadowRenderPass>();
	RenderPass* passes[] = {mShadowRenderPass.get()};
	shadowEnabled = gGraphic.RegisterRenderPasses(passes);
	return shadowEnabled;
}

void  LightManager::Initialize() {
	ID3D12Device* mDevice = gGraphic.GetDevice();
	mLightPass = std::make_unique<ConstantBuffer<LightPass>>(mDevice);
	mLightPass->GetBufferPtr()->ambient = Game::ConstColor::White;
	LightData& mainLight = mLightPass->GetBufferPtr()->lights[mainLightIndex];
	mainLight.direction = Game::Vector3(0., -1., 0.);
	mainLight.intensity = Game::Vector3(1., 1., 1.);
	mainLight.fallStart = 1.;
	mainLight.fallEnd = 100.;
	mainLight.type = SHADER_LIGHT_TYPE_DIRECTIONAL;
}