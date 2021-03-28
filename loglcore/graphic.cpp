#include "graphic.h"
#include "LightManager.h"

#include "DescriptorAllocator.h"
#include "TextureManager.h"
#include "GraphicUtil.h"

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"d3dcompiler.lib")

bool Graphic::createCommandObject() {
	D3D12_COMMAND_QUEUE_DESC desc = {};
	desc.NodeMask = 0;
	desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	desc.Flags = D3D12_COMMAND_QUEUE_FLAG_DISABLE_GPU_TIMEOUT;

	HRESULT hr = mDevice->CreateCommandQueue(&desc,IID_PPV_ARGS(&mCommandQueue));
	if (FAILED(hr)) {
		OUTPUT_DEBUG_STRING("fail to create the command queue\n");
		return false;
	}

	hr = mDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&mDrawCmdAlloc));
	if (FAILED(hr)) {
		OUTPUT_DEBUG_STRING("fail to create the draw command allocator\n");
		return false;
	}
	mDevice->CreateCommandList(0,D3D12_COMMAND_LIST_TYPE_DIRECT, 
		mDrawCmdAlloc.Get(),nullptr,IID_PPV_ARGS(&mDrawCmdList));
	if (FAILED(hr)) {
		OUTPUT_DEBUG_STRING("fail to create the draw command list");
		return false;
	}
	mDrawCmdList->Close();

	return true;
}

bool Graphic::createSwapChain() {
	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
	msQualityLevels.Format = mBackBufferFormat;
	msQualityLevels.SampleCount = 4;
	msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	msQualityLevels.NumQualityLevels = 0;
	HRESULT hr = mDevice->CheckFeatureSupport(
		D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
		&msQualityLevels,
		sizeof(msQualityLevels));
	if (FAILED(hr)) {
		OUTPUT_DEBUG_STRING("fail to query the maximun quality level number\n");
		return false;
	}

	massQuality = msQualityLevels.NumQualityLevels;

	DXGI_SWAP_CHAIN_DESC swDesc{};
	swDesc.BufferCount = Graphic_mBackBufferNum;
	
	swDesc.BufferDesc.Format = mBackBufferFormat;
	swDesc.BufferDesc.Height = mWinHeight;
	swDesc.BufferDesc.Width = mWinWidth;
	swDesc.BufferDesc.RefreshRate = {60 , 1};
	swDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	
	swDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	swDesc.Windowed = true;
	swDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swDesc.SampleDesc.Count = useMass ? 4 : 1;
	swDesc.SampleDesc.Quality = useMass ? (massQuality - 1) : 0;
	swDesc.OutputWindow = winHandle;

	hr = mDxgiFactory->CreateSwapChain(mCommandQueue.Get(), &swDesc, mDxgiSwapChain.GetAddressOf());
	if (FAILED(hr)) {
		OUTPUT_DEBUG_STRING("fail to create swap chain\n");
		return false;
	}
	return true;
}

bool Graphic::createRTV_DSV() {
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc{};

	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.NodeMask = 0;
	rtvHeapDesc.NumDescriptors = Graphic_mBackBufferNum * 2;

	HRESULT hr = mDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&mBackBufferRTVHeap));
	if (FAILED(hr)) {
		OUTPUT_DEBUG_STRING("fail to create rtv heap\n");
		return false;
	}

	/*
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.NodeMask = 0;
	srvHeapDesc.NumDescriptors = Graphic_mBackBufferNum;

	hr = mDevice->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&mSRVHeap));
	if (FAILED(hr)) {
		OUTPUT_DEBUG_STRING("fail to create srv heap for tone mapping\n");
		return false;
	}*/

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(mBackBufferRTVHeap->GetCPUDescriptorHandleForHeapStart());
	for (size_t i = 0; i != 3; i++) {
		mDxgiSwapChain->GetBuffer(i, IID_PPV_ARGS(&mBackBuffers[i]));

		mDevice->CreateRenderTargetView(mBackBuffers[i].Get(), nullptr, rtvHandle);
		rtvHandle.Offset(1, mDescriptorHandleSizeRTV);
	}


	/*D3D12_RESOURCE_DESC rtvResDesc;
	rtvResDesc.Alignment = 0;
	rtvResDesc.DepthOrArraySize = 1;
	rtvResDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	rtvResDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	rtvResDesc.Format = mRenderTargetFormat;
	rtvResDesc.Width = GetScreenWidth();
	rtvResDesc.Height = GetScreenHeight();
	rtvResDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	rtvResDesc.MipLevels = 1;
	rtvResDesc.SampleDesc.Count = 1;
	rtvResDesc.SampleDesc.Quality = 0;

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
	rtvDesc.Format = GetRenderTargetFormat();
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D = { 0,0 };
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = GetRenderTargetFormat();
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.PlaneSlice = 0;
	srvDesc.Texture2D.ResourceMinLODClamp = -1;

	D3D12_CLEAR_VALUE cv;
	cv.Format = mRenderTargetFormat;
	memcpy(cv.Color, mRTVClearColor, sizeof(mRTVClearColor));


	CD3DX12_CPU_DESCRIPTOR_HANDLE srvHandle(mSRVHeap->GetCPUDescriptorHandleForHeapStart());
	rtvHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(mBackBufferRTVHeap->GetCPUDescriptorHandleForHeapStart());
	rtvHandle.Offset(Graphic_mBackBufferNum, mDescriptorHandleSizeRTV);
	for (size_t i = 0; i != 3;i++) {
		hr = mDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&rtvResDesc,
			D3D12_RESOURCE_STATE_COMMON,
			&cv,
			IID_PPV_ARGS(&mRenderTargets[i]));
		if (FAILED(hr)) {
			OUTPUT_DEBUG_STRING("fail to create render target buffers\n");
			return false;
		}

		mDevice->CreateRenderTargetView(mRenderTargets[i].Get(),&rtvDesc,rtvHandle);
		mDevice->CreateShaderResourceView(mRenderTargets[i].Get(), &srvDesc,srvHandle);
		rtvHandle.Offset(1, mDescriptorHandleSizeRTV);
		srvHandle.Offset(1, mDescriptorHandleSizeCBVSRVUAV);
	}*/

	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc{};

	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsvHeapDesc.NodeMask = 0;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.NumDescriptors = 1;

	hr = mDevice->CreateDescriptorHeap(&dsvHeapDesc,IID_PPV_ARGS(&mBackBufferDSVhHeap));

	D3D12_RESOURCE_DESC dsDesc{};

	dsDesc.Alignment = 0;
	dsDesc.DepthOrArraySize = 1;
	dsDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	dsDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	dsDesc.Format = mBackBufferDepthFormat;
	dsDesc.Width = mWinWidth;
	dsDesc.Height = mWinHeight;
	dsDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	dsDesc.MipLevels = 1;
	dsDesc.SampleDesc.Quality = 0;
	dsDesc.SampleDesc.Count = 1;

	D3D12_CLEAR_VALUE dsvClear;
	dsvClear.DepthStencil.Depth = 1.0f;
	dsvClear.DepthStencil.Stencil = 0;
	dsvClear.Format = mBackBufferDepthFormat;

	hr = mDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&dsDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&dsvClear,
		IID_PPV_ARGS(&mDepthStencilBuffer));

	if (FAILED(hr)) {
		OUTPUT_DEBUG_STRING("fail to create depth buffer\n");
		return false;
	}

	mDevice->CreateDepthStencilView(mDepthStencilBuffer.Get(), nullptr, mBackBufferDSVhHeap->GetCPUDescriptorHandleForHeapStart());

	/*Game::RootSignature rootSig(2,1);
	rootSig[0].initAsDescriptorTable(0, 0, 1, D3D12_DESCRIPTOR_RANGE_TYPE_SRV);
	rootSig[1].initAsConstants(0, 0, 1);
	rootSig.InitializeSampler(0, CD3DX12_STATIC_SAMPLER_DESC(0));

	if (!CreateRootSignature(L"HDR",&rootSig)) {
		OUTPUT_DEBUG_STRING("fail to create root signature for hdr\n");
		return false;
	}

	std::vector<D3D12_INPUT_ELEMENT_DESC> input{};
	Shader* shader = gShaderManager.loadShader(L"../shader/Legacy/ToneMap.hlsl",
		"VS", "PS", L"HDR", input, L"HDR");
	if (shader == nullptr) {
		OUTPUT_DEBUG_STRING("fail to create shader for hdr\n");
		return false;
	}

	Game::GraphicPSO pso;
	pso.LazyBlendDepthRasterizeDefault();
	
	CD3DX12_DEPTH_STENCIL_DESC ds(D3D12_DEFAULT);
	ds.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
	ds.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	pso.SetDepthStencilState(ds);
	CD3DX12_RASTERIZER_DESC rd(D3D12_DEFAULT);
	rd.CullMode = D3D12_CULL_MODE_NONE;
	pso.SetRasterizerState(rd);

	pso.SetSampleMask(UINT_MAX);
	pso.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
	pso.SetFlag(D3D12_PIPELINE_STATE_FLAG_NONE);
	pso.SetRenderTargetFormat(mBackBufferFormat);
	pso.SetDepthStencilViewFomat(mBackBufferDepthFormat);

	if (!CreatePipelineStateObject(shader,&pso,L"HDR")) {
		OUTPUT_DEBUG_STRING("fail to create pso for hdr");
		return false;
	}
	*/

	return true;
}

bool Graphic::createShaderAndRootSignatures() {
	Game::RootSignature Default3DRootSigWithoutLight(2,0);
	Default3DRootSigWithoutLight[0].initAsConstantBuffer(0, 0, D3D12_SHADER_VISIBILITY_ALL);
	Default3DRootSigWithoutLight[1].initAsConstantBuffer(1, 0, D3D12_SHADER_VISIBILITY_ALL);

	if (!Default3DRootSigWithoutLight.EndEditingAndCreate(mDevice.Get())) {
		OUTPUT_DEBUG_STRING("ERROR : fail to create DefaultWithoutLight root signature");
		return false;
	}

	D3D_SHADER_MACRO WithOutLightMacro[] = {
		{"DISABLE_LIGHT_PASS","1"},
		{NULL				 ,NULL}
	};

	std::vector<D3D12_INPUT_ELEMENT_DESC> WithOutLightInputElement = {
		{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
		{"NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,12,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
		{"TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,24,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0}
	};

	Shader* result;
	if (result = gShaderManager.loadShader(L"../shader/Legacy/Default3D.hlsl", "VS", "PS",
		L"with_out_light", WithOutLightInputElement, L"with_out_light", WithOutLightMacro);
		result == nullptr) {
		OUTPUT_DEBUG_STRINGA("fail to load with_out_light shader\n");
		return false;
	}

	mRootSignatures[result->name] = Default3DRootSigWithoutLight.GetRootSignature();

	Game::GraphicPSO WithoutLightPso;
	WithoutLightPso.SetInputElementDesc(result->inputLayout);
	WithoutLightPso.LazyBlendDepthRasterizeDefault();
	CD3DX12_RASTERIZER_DESC rsDesc(D3D12_DEFAULT);
	WithoutLightPso.SetRasterizerState(rsDesc);
	WithoutLightPso.SetDepthStencilViewFomat(mBackBufferDepthFormat);
	WithoutLightPso.SetFlag(D3D12_PIPELINE_STATE_FLAG_NONE);
	WithoutLightPso.SetNodeMask(0);
	WithoutLightPso.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
	WithoutLightPso.SetRenderTargetFormat(mBackBufferFormat);
	WithoutLightPso.SetRootSignature(&Default3DRootSigWithoutLight);
	if (useMass) {
		WithoutLightPso.SetSampleDesc(4, massQuality - 1);
	}
	else {
		WithoutLightPso.SetSampleDesc(1, 0);
	}
	WithoutLightPso.SetSampleMask(UINT_MAX);

	WithoutLightPso.SetVertexShader(result->shaderByteCodeVS->GetBufferPointer(), result->shaderByteSizeVS);
	WithoutLightPso.SetPixelShader(result->shaderByteCodePS->GetBufferPointer(), result->shaderByteSizePS);

	if (!WithoutLightPso.Create(mDevice.Get())) {
		OUTPUT_DEBUG_STRING("fail to create pso for with out light pipeline\n");
		return false;
	}
	mPsos[result->name] = WithoutLightPso.GetPSO();

	return true;
}
/*
bool Graphic::createSpriteRenderingPipeline() {
	Game::RootSignature RootSig(3, 1);
	RootSig[0].initAsConstantBuffer(0, 0, D3D12_SHADER_VISIBILITY_ALL);
	RootSig[1].initAsConstantBuffer(1, 0, D3D12_SHADER_VISIBILITY_ALL);
	RootSig[2].initAsDescriptorTable(0, 0, 1, D3D12_DESCRIPTOR_RANGE_TYPE_SRV);
	RootSig.InitializeSampler(0, CD3DX12_STATIC_SAMPLER_DESC(0));

	if (!RootSig.EndEditingAndCreate(mDevice.Get())) {
		OUTPUT_DEBUG_STRING("fail to create root signature for sprite pipeline\n");
		return false;
	}

	std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout = {
		{"POSITION",0,DXGI_FORMAT_R32G32_FLOAT,0,0,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
		{"TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,8,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0}
	};

	Shader* shader = gShaderManager.loadShader(L"../shader/DrawSprite.hlsl","VS","PS",L"Sprite",
		inputLayout,L"DrawSprite");
	if (shader == nullptr) {
		OUTPUT_DEBUG_STRING("fail to create shader for sprite rendering\n");
		return false;
	}

	Game::GraphicPSO Pso;
	Pso.SetInputElementDesc(shader->inputLayout);
	Pso.LazyBlendDepthRasterizeDefault();
	CD3DX12_RASTERIZER_DESC rsDesc(D3D12_DEFAULT);
	rsDesc.CullMode = D3D12_CULL_MODE_NONE;
	Pso.SetRasterizerState(rsDesc);


	Pso.SetDepthStencilViewFomat(mBackBufferDepthFormat);
	Pso.SetFlag(D3D12_PIPELINE_STATE_FLAG_NONE);
	Pso.SetNodeMask(0);
	Pso.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
	Pso.SetRenderTargetFormat(mBackBufferFormat);
	Pso.SetRootSignature(&RootSig);
	if (useMass) {
		Pso.SetSampleDesc(4, 1);
	}
	else {
		Pso.SetSampleDesc(1, 0);
	}
	Pso.SetSampleMask(UINT_MAX);

	Pso.SetVertexShader(shader->shaderByteCodeVS->GetBufferPointer(), shader->shaderByteSizeVS);
	Pso.SetPixelShader(shader->shaderByteCodePS->GetBufferPointer(), shader->shaderByteSizePS);

	if (!Pso.Create(mDevice.Get())) {
		OUTPUT_DEBUG_STRING("fail to create pso for with out light pipeline\n");
		return false;
	}
	mPsos[shader->name] = Pso.GetPSO();
	mRootSignatures[L"Sprite"] = RootSig.GetRootSignature();

	return true;
}
*/

bool Graphic::initialize(HWND winHnd, size_t width, size_t height){
#ifdef _DEBUG
	{
		ComPtr<ID3D12Debug> debug;
		D3D12GetDebugInterface(IID_PPV_ARGS(&debug));
		debug->EnableDebugLayer();
		debug = nullptr;
	}
#endif

	mWinHeight = height, mWinWidth = width;
	winHandle = winHnd;

	viewPort.Width = width;
	viewPort.Height = height;
	viewPort.TopLeftX = 0;
	viewPort.TopLeftY = 0;
	viewPort.MinDepth = 0;
	viewPort.MaxDepth = 1.0f;

	sissorRect.bottom = height;
	sissorRect.top = 0;
	sissorRect.left = 0;
	sissorRect.right = width;
	
	HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&mDxgiFactory));
	if (FAILED(hr)) {
		OUTPUT_DEBUG_STRING("ERROR : fail to create d3d dxgi factory\n");
		return false;
	}
	if (!createDevice()) {
		OUTPUT_DEBUG_STRING("ERROR : fail to create device \n");
		return false;

	}
	mDescriptorHandleSizeCBVSRVUAV = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	mDescriptorHandleSizeRTV = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	mDescriptorHandleSizeDSV = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);


	if(!createCommandObject()){
		OUTPUT_DEBUG_STRING("ERROR : fail to create command object");
		return false;
	}

	if (!createSwapChain()) {
		OUTPUT_DEBUG_STRING("ERROR : fail to create swap chain\n");
		return false;
	}

	if (!createRTV_DSV()) {
		OUTPUT_DEBUG_STRING("ERROR : fail to create rtv dsv\n");
		return false;
	}
	if (!createShaderAndRootSignatures()) {
		OUTPUT_DEBUG_STRING("ERROR : fail to create shaders and root signatures\n");
		return false;
	}
	if (!createRenderPasses()) {
		OUTPUT_DEBUG_STRING("ERROR : fail to create default render passes\n");
		return false;
	}
	fenceEvent = CreateEventEx(NULL, NULL, NULL, EVENT_ALL_ACCESS);
	if (fenceEvent == NULL) {
		OUTPUT_DEBUG_STRING("ERROR : fail to create fence event\n");
		return false;
	}
	hr = mDevice->CreateFence(0,D3D12_FENCE_FLAG_NONE,IID_PPV_ARGS(&mFence));
	if (FAILED(hr)) {
		OUTPUT_DEBUG_STRING("ERROR : fail to create fence\n");
		return false;
	}

	mainCamera.initialize(45.,(float)width / (float)height,
		0.1,1000.,Game::Vector3(0.,0.,0.),Game::Vector3(0.,0.,0.));
	
	cameraPassData = std::make_unique<ConstantBuffer<CameraPass>>(mDevice.Get());

	cameraPassData->GetBufferPtr()->cameraPos = mainCamera.getPosition();
	cameraPassData->GetBufferPtr()->viewMat = mainCamera.getViewMat().T();
	cameraPassData->GetBufferPtr()->perspectMat = mainCamera.getPerspectMat().T();
	cameraPassData->GetBufferPtr()->exposure = exposure;

	state = READY;

	gLightManager.Initialize();

	return true;
}

static size_t ScoreAdapter(IDXGIAdapter* adapter) {
	DXGI_ADAPTER_DESC adaDesc;
	adapter->GetDesc(&adaDesc);
	//À¬»øÎ¢Èíguna
	if (std::wstring(adaDesc.Description) == L"Microsoft Basic Render Driver") {
		return 0;
	}
	return adaDesc.DedicatedSystemMemory + adaDesc.SharedSystemMemory + adaDesc.DedicatedVideoMemory;
}

bool Graphic::createDevice() {

	ComPtr<IDXGIAdapter> adapter,targetAda = nullptr;
	size_t iadapter = 0,score = 0;
	while ( mDxgiFactory->EnumAdapters(iadapter, &adapter) != DXGI_ERROR_NOT_FOUND) {
		size_t mScore = ScoreAdapter(adapter.Get());
		if (mScore > score) {
			targetAda = adapter;
			score = mScore;
		}
		iadapter++;
	}

	HRESULT hr;

	if (targetAda == nullptr) {
		hr = D3D12CreateDevice(nullptr,
			D3D_FEATURE_LEVEL_12_0,
			IID_PPV_ARGS(&mDevice));
	}
	else {
		hr = D3D12CreateDevice(targetAda.Get(),
			D3D_FEATURE_LEVEL_12_0,
			IID_PPV_ARGS(&mDevice));
	}

	if (FAILED(hr)) {
		return false;
	}

	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
	msQualityLevels.Format = mBackBufferFormat;
	msQualityLevels.SampleCount = 4;
	msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	msQualityLevels.NumQualityLevels = 0;
	mDevice->CheckFeatureSupport(
		D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
		&msQualityLevels,
		sizeof(msQualityLevels));

	massQuality = msQualityLevels.NumQualityLevels;
	return true;
}

void Graphic::FlushCommandQueue() {
	fenceValue++;
	mCommandQueue->Signal(mFence.Get(), fenceValue);
	if (mFence->GetCompletedValue() < fenceValue) {
		if (FAILED(mFence->SetEventOnCompletion(fenceValue,fenceEvent))) {
			OUTPUT_DEBUG_STRING("fail to set wait event for completion\n");
			exit(-1);
		}
		WaitForSingleObject(fenceEvent, INFINITE);
	}
}

void Graphic::begin() {
	if (state != READY) return;
	state = BEGIN_COMMAND_RECORDING;

	mDrawCmdAlloc->Reset();
	mDrawCmdList->Reset(mDrawCmdAlloc.Get(),nullptr);

	UpdateCameraPass();

	/*
	D3D12_RESOURCE_BARRIER barriers[2] = {
		CD3DX12_RESOURCE_BARRIER::Transition(mBackBuffers[mCurrBackBuffer].Get(),
		D3D12_RESOURCE_STATE_PRESENT,
		D3D12_RESOURCE_STATE_RENDER_TARGET),
		CD3DX12_RESOURCE_BARRIER::Transition(mRenderTargets[mCurrBackBuffer].Get(),
		D3D12_RESOURCE_STATE_COMMON,
		D3D12_RESOURCE_STATE_RENDER_TARGET)
	};
	mDrawCmdList->ResourceBarrier(2,barriers);
	*/

	mDrawCmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mBackBuffers[mCurrBackBuffer].Get(),
		D3D12_RESOURCE_STATE_PRESENT,
		D3D12_RESOURCE_STATE_RENDER_TARGET));

	mDrawCmdList->RSSetScissorRects(1, &sissorRect);
	mDrawCmdList->RSSetViewports(1, &viewPort);

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtv(mBackBufferRTVHeap->GetCPUDescriptorHandleForHeapStart())
		,dsv(mBackBufferDSVhHeap->GetCPUDescriptorHandleForHeapStart());

	rtv.Offset(mCurrBackBuffer, mDescriptorHandleSizeRTV);
	mDrawCmdList->OMSetRenderTargets(1,&rtv,true,&dsv);
	mDrawCmdList->ClearRenderTargetView(rtv,mRTVClearColor,0,nullptr);
	mDrawCmdList->ClearDepthStencilView(mBackBufferDSVhHeap->GetCPUDescriptorHandleForHeapStart(), 
		D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
		1.f, 0, 0, nullptr);

}

void Graphic::end() {
	if (state != BEGIN_COMMAND_RECORDING) return;

	for (auto& rps : RPQueue) {
		for (auto& RP : rps.second) {
			RP->PreProcess();
		}
	}

	auto renderLayer = [&](RENDER_PASS_LAYER layer) {
		for (auto& rps : RPQueue) {
			for (auto& RP : rps.second) {
				RP->Render(this, layer);
			}
		}
	};

	renderLayer(RENDER_PASS_LAYER_BEFORE_ALL);
	renderLayer(RENDER_PASS_LAYER_OPAQUE);
	renderLayer(RENDER_PASS_LAYER_TRANSPARENT);
	renderLayer(RENDER_PASS_LAYER_AFTER_ALL);

	for (auto& rps : RPQueue) {
		for (auto& RP : rps.second) {
			RP->PostProcess(mBackBuffers[mCurrBackBuffer].Get());
		}
	}
	
	/*mDrawCmdList->ResourceBarrier(
		1,&CD3DX12_RESOURCE_BARRIER::Transition(mBackBuffers[mCurrBackBuffer].Get(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_COMMON)
	);*/

	/*
	mDrawCmdList->SetPipelineState(mPsos[L"HDR"].Get());
	mDrawCmdList->SetGraphicsRootSignature(mRootSignatures[L"HDR"].Get());

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtv(mBackBufferRTVHeap->GetCPUDescriptorHandleForHeapStart()),
		dsv(mBackBufferDSVhHeap->GetCPUDescriptorHandleForHeapStart());
	rtv.Offset(mCurrBackBuffer, mDescriptorHandleSizeRTV);
	mDrawCmdList->OMSetRenderTargets(1,&rtv,false,&dsv);

	ID3D12DescriptorHeap* heaps[] = {mSRVHeap.Get()};
	mDrawCmdList->SetDescriptorHeaps(_countof(heaps), heaps);

	mDrawCmdList->SetGraphicsRootDescriptorTable(0, 
		CD3DX12_GPU_DESCRIPTOR_HANDLE(mSRVHeap->GetGPUDescriptorHandleForHeapStart()).Offset(mCurrBackBuffer,mDescriptorHandleSizeRTV)
	);
	mDrawCmdList->SetGraphicsRoot32BitConstant(1, Pack32bitNum(exposure), 0);

	mDrawCmdList->IASetVertexBuffers(0, 0, nullptr);
	mDrawCmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	mDrawCmdList->DrawInstanced(6, 1, 0, 0);*/
	
	mDrawCmdList->ResourceBarrier(1,
		&CD3DX12_RESOURCE_BARRIER::Transition(mBackBuffers[mCurrBackBuffer].Get(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PRESENT));

	mDrawCmdList->Close();

	ID3D12CommandList* toExcute[] = { mDrawCmdList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(toExcute), toExcute);

	mDxgiSwapChain->Present(0, 0);
	mCurrBackBuffer = (mCurrBackBuffer + 1) % Graphic_mBackBufferNum;

	FlushCommandQueue();
	
	state = READY;
}

void Graphic::finalize() {
	FlushCommandQueue();

	for (auto[name, rp] : RPQueue) {
		for (auto item : rp) item->finalize();
	}
	gDescriptorAllocator.finalize();

	cameraPassData.release();

	mRootSignatures.clear();
	mPsos.clear();

	mBackBufferDSVhHeap = nullptr;
	mBackBufferRTVHeap = nullptr;

	for (size_t i = 0; i != Graphic_mBackBufferNum;i++) {
		mBackBuffers[i] = nullptr;
	}
	mDepthStencilBuffer = nullptr;

	mCommandQueue = nullptr;
	mDrawCmdList = nullptr;
	mDrawCmdAlloc = nullptr;

	mFence = nullptr;
	mDxgiFactory = nullptr;
	mDxgiSwapChain = nullptr;
	mDevice = nullptr;
}

void Graphic::BindShader(Shader* shader) {
	if (state != BEGIN_COMMAND_RECORDING) return;
	ID3D12PipelineState* pipelineState = mPsos[shader->name].Get();
	ID3D12RootSignature* rootSig = mRootSignatures[shader->rootSignatureName].Get();

	mDrawCmdList->SetPipelineState(pipelineState);
	mDrawCmdList->SetGraphicsRootSignature(rootSig);
}

bool Graphic::BindPSOAndRootSignature(const wchar_t* psoName,const wchar_t* rootSigName) {
	if (state != BEGIN_COMMAND_RECORDING) return false;
	ID3D12PipelineState* pso;
	if (auto query = mPsos.find(psoName);query == mPsos.end()) {
		return false;
	}
	else {
		pso = query->second.Get();
	}
	ID3D12RootSignature* rootSig;
	if (auto query = mRootSignatures.find(rootSigName);query != mRootSignatures.end()) {
		rootSig = query->second.Get();
	}
	else {
		return false;
	}

	mDrawCmdList->SetPipelineState(pso);
	mDrawCmdList->SetGraphicsRootSignature(rootSig);
	return true;
}

void Graphic::BindConstantBuffer(ID3D12Resource* res, size_t slot,size_t offset) {
	if (state != BEGIN_COMMAND_RECORDING) return;
	
	mDrawCmdList->SetGraphicsRootConstantBufferView(slot,res->GetGPUVirtualAddress() + offset);
}

void Graphic::BindShaderResource(ID3D12Resource* res, size_t slot,size_t offset) {
	if (state != BEGIN_COMMAND_RECORDING) return;

	mDrawCmdList->SetGraphicsRootShaderResourceView(slot, res->GetGPUVirtualAddress() + offset);
}

void Graphic::BindConstantBuffer(D3D12_GPU_VIRTUAL_ADDRESS vaddr, size_t slot) {
	if (state != BEGIN_COMMAND_RECORDING) return;

	mDrawCmdList->SetGraphicsRootConstantBufferView(slot, vaddr);
}

void Graphic::BindShaderResource(D3D12_GPU_VIRTUAL_ADDRESS vaddr, size_t slot) {
	if (state != BEGIN_COMMAND_RECORDING) return;
	
	mDrawCmdList->SetGraphicsRootShaderResourceView(slot, vaddr);
}

void Graphic::BindMainCameraPass(size_t slot) {
	if (state == BEGIN_COMMAND_RECORDING) {
		CameraPass* data = cameraPassData->GetBufferPtr();
		mDrawCmdList->SetGraphicsRootConstantBufferView(slot, cameraPassData->GetADDR());
	}
}

void Graphic::UpdateCameraPass() {
	cameraPassData->GetBufferPtr()->cameraPos = mainCamera.getPosition();
	cameraPassData->GetBufferPtr()->viewMat = mainCamera.getViewMat().T();
	cameraPassData->GetBufferPtr()->perspectMat = mainCamera.getPerspectMat().T();

	cameraPassData->GetBufferPtr()->transInvView = mainCamera.getViewMat().R();
}

void Graphic::Draw(D3D12_VERTEX_BUFFER_VIEW* vbv, size_t start, size_t num, D3D_PRIMITIVE_TOPOLOGY topolgy) {
	if (state != BEGIN_COMMAND_RECORDING) return;
	
	mDrawCmdList->IASetPrimitiveTopology(topolgy);
	if (vbv != nullptr)
		mDrawCmdList->IASetVertexBuffers(0, 1, vbv);
	else
		mDrawCmdList->IASetVertexBuffers(0, 0, nullptr);
	mDrawCmdList->DrawInstanced(num, 1, start, 0);
}

void Graphic::Draw(D3D12_VERTEX_BUFFER_VIEW* vbv, D3D12_INDEX_BUFFER_VIEW* ibv, size_t start, size_t num, D3D_PRIMITIVE_TOPOLOGY topolgy) {
	if (state != BEGIN_COMMAND_RECORDING) return;

	mDrawCmdList->IASetPrimitiveTopology(topolgy);

	mDrawCmdList->IASetIndexBuffer(ibv);
	mDrawCmdList->IASetVertexBuffers(0, 1, vbv);
	mDrawCmdList->DrawIndexedInstanced(num, 1, start, 0, 0);
}

void Graphic::DrawInstance(D3D12_VERTEX_BUFFER_VIEW* vbv, size_t start, size_t num, size_t instanceNum, D3D_PRIMITIVE_TOPOLOGY topolgy) {
	if (state != BEGIN_COMMAND_RECORDING) return;

	mDrawCmdList->IASetPrimitiveTopology(topolgy);
	mDrawCmdList->IASetVertexBuffers(0, 1, vbv);
	mDrawCmdList->DrawInstanced(num, instanceNum, start, 0);
}

void Graphic::DrawInstance(D3D12_VERTEX_BUFFER_VIEW* vbv, D3D12_INDEX_BUFFER_VIEW* ibv, size_t start, size_t num, size_t instanceNum, D3D_PRIMITIVE_TOPOLOGY topolgy) {
	if (state != BEGIN_COMMAND_RECORDING) return;

	mDrawCmdList->IASetPrimitiveTopology(topolgy);

	mDrawCmdList->IASetIndexBuffer(ibv);
	mDrawCmdList->IASetVertexBuffers(0, 1, vbv);
	mDrawCmdList->DrawIndexedInstanced(num, instanceNum, start, 0, 0);
}

void Graphic::onResize(size_t width, size_t height) {
	viewPort.Width = width;
	viewPort.Height = height;
	viewPort.TopLeftX = 0;
	viewPort.TopLeftY = 0;
	viewPort.MinDepth = 0;
	viewPort.MaxDepth = 1.0f;

	sissorRect.bottom = height;
	sissorRect.top = 0;
	sissorRect.left = 0;
	sissorRect.right = width;

	FlushCommandQueue();

	for (int i = 0; i != Graphic_mBackBufferNum; i++) {
		mBackBuffers[i] = nullptr;
	}

	HRESULT hr = mDxgiSwapChain->ResizeBuffers(Graphic_mBackBufferNum,
		width, height,
		mBackBufferFormat, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);
	if (FAILED(hr)) {
		OUTPUT_DEBUG_STRING("fail to resize the swapchains\n");
		exit(-1);
	}

	//mCurrentFrameIndex = mDxgiSwapChain->GetCurrentBackBufferIndex();

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(mBackBufferRTVHeap->GetCPUDescriptorHandleForHeapStart());

	for (int i = 0; i != Graphic_mBackBufferNum; i++) {
		mDxgiSwapChain->GetBuffer(i, IID_PPV_ARGS(&mBackBuffers[i]));

		mDevice->CreateRenderTargetView(mBackBuffers[i].Get(), nullptr, rtvHandle);
		rtvHandle.Offset(1, mDescriptorHandleSizeRTV);
	}

	D3D12_RESOURCE_DESC depthDesc;
	depthDesc.Format = mBackBufferDepthFormat;
	depthDesc.Alignment = 0;
	depthDesc.DepthOrArraySize = 1;
	depthDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	depthDesc.Height = height;
	depthDesc.Width = width;
	depthDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthDesc.MipLevels = 1;
	depthDesc.SampleDesc.Count = 1;
	depthDesc.SampleDesc.Quality = 0;

	mDepthStencilBuffer = nullptr;
	//We assume that the default depth stencil compare function is less so the 
	//default value of the depth buffer is 1.0f
	D3D12_CLEAR_VALUE cValue = {};
	cValue.Format = mBackBufferDepthFormat;
	cValue.DepthStencil.Depth = 1.f;
	cValue.DepthStencil.Stencil = 0;

	hr = mDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE, &depthDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&cValue, IID_PPV_ARGS(&mDepthStencilBuffer));

	if (FAILED(hr)) {
		OUTPUT_DEBUG_STRING("fail to create new depth buffer for resized buffers\n");
		exit(-1);
	}

	mDevice->CreateDepthStencilView(mDepthStencilBuffer.Get(), nullptr,
		mBackBufferDSVhHeap->GetCPUDescriptorHandleForHeapStart());
}

void Graphic::BindDescriptorHandle(D3D12_GPU_DESCRIPTOR_HANDLE handle,size_t slot) {
	if (state != BEGIN_COMMAND_RECORDING) {
		return;
	}
	mDrawCmdList->SetGraphicsRootDescriptorTable(slot, handle);
}

void Graphic::BindDescriptorHeap(ID3D12DescriptorHeap* const* heap,size_t heap_num) {
	if (state != BEGIN_COMMAND_RECORDING) {
		return;
	}
	mDrawCmdList->SetDescriptorHeaps(heap_num, heap);
}

void Graphic::BindDescriptorHeap(ID3D12DescriptorHeap* heap) {
	ID3D12DescriptorHeap* heaps[] = {heap};
	BindDescriptorHeap(heaps, 1);
}

bool Graphic::CreatePipelineStateObject(Shader* shader,Game::GraphicPSO* PSO,const wchar_t* name,bool rp) {
	ID3D12PipelineState* mPSO;
	auto iter = mRootSignatures.find(shader->rootSignatureName);
	if (iter == mRootSignatures.end()) {
		return false;
	}
	PSO->SetRootSignature(iter->second.Get());
	if (useMass) {
		PSO->SetSampleDesc(4, massQuality);
	}
	else {
		PSO->SetSampleDesc(1, 0);
	}
	PSO->SetVertexShader(shader->shaderByteCodeVS->GetBufferPointer(), shader->shaderByteSizeVS);
	PSO->SetPixelShader(shader->shaderByteCodePS->GetBufferPointer(), shader->shaderByteSizePS);
	PSO->SetInputElementDesc(shader->inputLayout);
	PSO->SetSampleMask(UINT_MAX);
	if (rp) {
		PSO->SetRenderTargetFormat(GetRenderTargetFormat());
		PSO->SetDepthStencilViewFomat(GetDepthStencilFormat());
	}

	if (!PSO->Create(mDevice.Get())) {
		return false;
	}

	mPSO = PSO->GetPSO();
	if (name == nullptr)
		mPsos[shader->name] = mPSO;
	else
		mPsos[name] = mPSO;
	return true;
}

bool Graphic::CreateComputePipelineStateObject(ComputeShader* shader,Game::ComputePSO* PSO,const wchar_t* name) {
	if (name == nullptr) name = shader->name.c_str();
	if (mPsos.find(name) != mPsos.end()) {
		return false;
	}
	auto rootSig = mRootSignatures.find(shader->rootSignatureName);
	if (rootSig == mRootSignatures.end()) {
		return false;
	}
	PSO->SetComputePipelineStateRootSignature(rootSig->second.Get());
	PSO->SetComputePipelineStateComputeShader(shader->shaderByteCodeCS->GetBufferPointer(),
		shader->shaderByteSizeCS);
	
	if (!PSO->Create(mDevice.Get())) {
		return false;
	}

	mPsos[name] = PSO->GetPSO();
	return true;
}

bool Graphic::CreateRootSignature(std::wstring name, Game::RootSignature* rootSig) {
	if (mRootSignatures.find(name) != mRootSignatures.end()) {
		return false;
	}
	if (!rootSig->EndEditingAndCreate(mDevice.Get())) {
		return false;
	}

	mRootSignatures[name] = rootSig->GetRootSignature();
	return true;
}

bool Graphic::RegisterRenderPasses(RenderPass** RP,size_t num) {
	UploadBatch mbatch = UploadBatch::Begin();
	for (size_t i = 0; i != num; i++) {
		if (!RP[i]->Initialize(&mbatch)) {
			RP[i]->finalize();
			return false;
		}
	}
	mbatch.End();

	for (size_t i = 0; i != num; i++) {
		size_t priority = RP[i]->GetPriority();
		auto iter = RPQueue.find(priority);
		if (iter == RPQueue.end()) {
			RPQueue[priority] = { RP[i] };
		}
		else {
			iter->second.push_back(RP[i]);
		}
	}

	return true;
}

bool Graphic::createRenderPasses() {
	spriteRenderPass = std::make_unique<SpriteRenderPass>();
	debugRenderPass = std::make_unique<DebugRenderPass>();
	skyboxRenderPass = std::make_unique<SkyboxRenderPass>();
	postProcessRenderPass = std::make_unique<PostProcessRenderPass>();
	toonRenderPass = std::make_unique<ToonRenderPass>();
	
	RenderPass* rpList[] = { spriteRenderPass.get(),debugRenderPass.get(),skyboxRenderPass.get(),postProcessRenderPass.get(),
		toonRenderPass.get()};
	return RegisterRenderPasses(rpList, _countof(rpList));
}

void Graphic::ResourceTransition(ID3D12Resource* resource,D3D12_RESOURCE_STATES initState,D3D12_RESOURCE_STATES afterState) {
	if (state != BEGIN_COMMAND_RECORDING) return;

	mDrawCmdList->ResourceBarrier(1,
		&CD3DX12_RESOURCE_BARRIER::Transition(
			resource,
			initState,
			afterState
		)
	);
}

void Graphic::ResourceTransition(ID3D12Resource** resource, D3D12_RESOURCE_STATES* initState, D3D12_RESOURCE_STATES* afterState,
	size_t num) {
	for (size_t i = 0; i != num; i++) {
		barriers.push_back(CD3DX12_RESOURCE_BARRIER::Transition(resource[i], initState[i], afterState[i]));
	}
	mDrawCmdList->ResourceBarrier(barriers.size(),barriers.data());
	barriers.clear();
}

void Graphic::ResourceCopy(ID3D12Resource* Dest, ID3D12Resource* Source) {
	if (state != BEGIN_COMMAND_RECORDING) return;

	mDrawCmdList->CopyResource(Dest, Source);
}

void Graphic::ResourceCopy(ID3D12Resource* Dest, ID3D12Resource* Source,
	D3D12_RESOURCE_STATES initDestState, 
	D3D12_RESOURCE_STATES initSourceState, 
	D3D12_RESOURCE_STATES destAfterState,
	D3D12_RESOURCE_STATES sourceAfterState) {
	if (state != BEGIN_COMMAND_RECORDING) return;

	if (initDestState != D3D12_RESOURCE_STATE_COPY_DEST) {
		mDrawCmdList->ResourceBarrier(1,
			&CD3DX12_RESOURCE_BARRIER::Transition(
				Dest,
				initDestState,
				D3D12_RESOURCE_STATE_COPY_DEST
			)
		);
	}

	if (initSourceState != D3D12_RESOURCE_STATE_COPY_SOURCE) {
		mDrawCmdList->ResourceBarrier(1,
			&CD3DX12_RESOURCE_BARRIER::Transition(
				Source,
				initSourceState,
				D3D12_RESOURCE_STATE_COPY_SOURCE
			)
		);
	}

	mDrawCmdList->CopyResource(Dest, Source);

	if (destAfterState != D3D12_RESOURCE_STATE_COPY_DEST) {
		mDrawCmdList->ResourceBarrier(1,
			&CD3DX12_RESOURCE_BARRIER::Transition(
				Dest, 
				D3D12_RESOURCE_STATE_COPY_DEST,
				destAfterState
			)
		);
	}

	if (sourceAfterState != D3D12_RESOURCE_STATE_COPY_DEST) {
		mDrawCmdList->ResourceBarrier(1,
			&CD3DX12_RESOURCE_BARRIER::Transition(
				Dest,
				D3D12_RESOURCE_STATE_COPY_DEST,
				sourceAfterState
			)
		);
	}
}


void Graphic::BindCurrentBackBufferAsRT(bool clear,float* clearValue) {
	if (state != BEGIN_COMMAND_RECORDING) return;
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtv(mBackBufferRTVHeap->GetCPUDescriptorHandleForHeapStart());
	rtv.Offset(mCurrBackBuffer);

	mDrawCmdList->OMSetRenderTargets(1, &rtv, true, nullptr);

	if (clear) {
		if (clearValue == nullptr) clearValue = mRTVClearColor;
		mDrawCmdList->ClearRenderTargetView(rtv, clearValue, 0, nullptr);
	}
}

void Graphic::BindCurrentBackBufferAsRTWidthDepth(bool clear, float* clearValue){
	if (state != BEGIN_COMMAND_RECORDING) return;
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtv(mBackBufferRTVHeap->GetCPUDescriptorHandleForHeapStart());
	rtv.Offset(mCurrBackBuffer, mDescriptorHandleSizeRTV);
	D3D12_CPU_DESCRIPTOR_HANDLE dsv = mBackBufferDSVhHeap->GetCPUDescriptorHandleForHeapStart();


	mDrawCmdList->OMSetRenderTargets(1, &rtv, true, &dsv);
	if (clear) {
		mDrawCmdList->ClearDepthStencilView(dsv,
			D3D12_CLEAR_FLAG_STENCIL | D3D12_CLEAR_FLAG_DEPTH,
			1.f, 0, 0, nullptr);
		if (clearValue == nullptr) {
			clearValue = mRTVClearColor;
		}
		mDrawCmdList->ClearRenderTargetView(
			rtv, clearValue, 0, nullptr
		);
	}
	mDrawCmdList->RSSetScissorRects(1, &sissorRect);
	mDrawCmdList->RSSetViewports(1, &viewPort);
}

void Graphic::BindRenderTarget(D3D12_CPU_DESCRIPTOR_HANDLE* rtvHandle,D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle,size_t rtvNum, 
	bool clear,float* clearValue, D3D12_VIEWPORT* viewPort, D3D12_RECT* rect,bool clearDepth) {
	if (state != BEGIN_COMMAND_RECORDING) return;
	if (dsvHandle.ptr == 0) dsvHandle = this->mBackBufferDSVhHeap->GetCPUDescriptorHandleForHeapStart();

	mDrawCmdList->OMSetRenderTargets(rtvNum, rtvHandle, true, &dsvHandle);
	if (clearDepth) {
		mDrawCmdList->ClearDepthStencilView(dsvHandle,
			D3D12_CLEAR_FLAG_STENCIL | D3D12_CLEAR_FLAG_DEPTH,
			1.f, 0, 0, nullptr);
	}
	if (clear) {	
		if (clearValue == nullptr) {
			clearValue = mRTVClearColor;
		}
		for (size_t i = 0; i != rtvNum;i++) {
			mDrawCmdList->ClearRenderTargetView(rtvHandle[i], clearValue, 0, nullptr);
		}
	}

	if (viewPort != nullptr) {
		mDrawCmdList->RSSetViewports(rtvNum, viewPort);
	}
	if (rect != nullptr) {
		mDrawCmdList->RSSetScissorRects(rtvNum, rect);
	}
}

void Graphic::SetSissorRect(D3D12_RECT* rect,size_t num) {
	if (state != BEGIN_COMMAND_RECORDING) {
		return;
	}
	mDrawCmdList->RSSetScissorRects(num, rect);
}

void Graphic::SetViewPort(D3D12_VIEWPORT* viewPort,size_t num) {
	if (state != BEGIN_COMMAND_RECORDING) {
		return;
	}
	mDrawCmdList->RSSetViewports(num, viewPort);
}

void Graphic::RestoreOriginalViewPortAndRect() {
	if (state != BEGIN_COMMAND_RECORDING) {
		return;
	}
	mDrawCmdList->RSSetViewports(1, &viewPort);
	mDrawCmdList->RSSetScissorRects(1, &sissorRect);
}

void Graphic::FindRPAndErase(RenderPass* rp) {
	for (auto iter = RPQueue.begin(); iter != RPQueue.end(); iter++) {
		bool finded = false;
		for (auto mrp = iter->second.begin(); mrp != iter->second.end();mrp++) {
			if (*mrp == rp) {
				iter->second.erase(mrp);
				finded = true;
				break;
			}
		}
		if (iter->second.empty()) {
			RPQueue.erase(iter);
		}
		if (finded) {
			break;
		}
	}
}