#include "graphic.h"

#include "RootSignature.h"
#include "PipelineStateObject.h"

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"d3dcompiler.lib")

bool Graphic::createCommandObject() {
	D3D12_COMMAND_QUEUE_DESC desc = {};
	desc.NodeMask = 0;
	desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

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
	swDesc.SampleDesc.Quality = useMass ? 1 : 0;
	swDesc.OutputWindow = winHandle;

	HRESULT hr = mDxgiFactory->CreateSwapChain(mCommandQueue.Get(), &swDesc, mDxgiSwapChain.GetAddressOf());
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
	rtvHeapDesc.NumDescriptors = Graphic_mBackBufferNum;

	HRESULT hr = mDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&mBackBufferRTVHeap));
	if (FAILED(hr)) {
		OUTPUT_DEBUG_STRING("fail to create rtv heap\n");
		return false;
	}

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(mBackBufferRTVHeap->GetCPUDescriptorHandleForHeapStart());
	mDescriptorHandleSizeRTV = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	for (size_t i = 0; i != 3; i++) {
		mDxgiSwapChain->GetBuffer(i, IID_PPV_ARGS(&mBackBuffers[i]));

		mDevice->CreateRenderTargetView(mBackBuffers[i].Get(), nullptr, rtvHandle);
		rtvHandle.Offset(1, mDescriptorHandleSizeRTV);
	}

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
	if (result = gShaderManager.loadShader(L"../shader/Default3D.hlsl", "VS", "PS",
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
	rsDesc.FillMode = D3D12_FILL_MODE_WIREFRAME;
	rsDesc.CullMode = D3D12_CULL_MODE_NONE;
	WithoutLightPso.SetRasterizerState(rsDesc);

	WithoutLightPso.SetDepthStencilViewFomat(mBackBufferDepthFormat);
	WithoutLightPso.SetFlag(D3D12_PIPELINE_STATE_FLAG_NONE);
	WithoutLightPso.SetNodeMask(0);
	WithoutLightPso.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
	WithoutLightPso.SetRenderTargetFormat(mBackBufferFormat);
	WithoutLightPso.SetRootSignature(&Default3DRootSigWithoutLight);
	if (useMass) {
		WithoutLightPso.SetSampleDesc(4, 1);
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

bool Graphic::initialize(HWND winHnd, size_t width, size_t height) {
#ifdef _DEBUG
	{
		ComPtr<ID3D12Debug> debug;
		D3D12GetDebugInterface(IID_PPV_ARGS(&debug));
		debug->EnableDebugLayer();
		debug->Release();
	}
#endif

	mWinHeight = height, mWinWidth = width;
	winHandle = winHnd;
	
	HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&mDxgiFactory));
	if (FAILED(hr)) {
		OUTPUT_DEBUG_STRING("ERROR : fail to create d3d dxgi factory\n");
		return false;
	}

	hr = D3D12CreateDevice(nullptr,
		D3D_FEATURE_LEVEL_12_0,
		IID_PPV_ARGS(&mDevice));

	if (FAILED(hr)) {
		OUTPUT_DEBUG_STRING("ERROR : fail to create device");
		return false;
	}

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
		OUTPUT_DEBUG_STRING("ERROR : fail to create shaders and root signatures");
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
	size_t cameraBufferSize = sizeof(CameraPassBufferData);
	cameraBufferSize = (cameraBufferSize + 255) & (~255);
	hr = mDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(cameraBufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&cameraPassBuffer)
	);
	if (FAILED(hr)) {
		OUTPUT_DEBUG_STRING("ERROR : fail to create main camera pass for graphic\n");
		return false;
	}
	void* ptr;
	cameraPassBuffer->Map(0, nullptr, &ptr);
	cameraPassBufferData = reinterpret_cast<CameraPassBufferData*>(ptr);

	cameraPassBufferData->cameraPos = mainCamera.getPosition();
	cameraPassBufferData->view = mainCamera.getViewMat().T();
	cameraPassBufferData->perspect = mainCamera.getPerspectMat().T();

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

	state = READY;

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

	mDrawCmdList->ResourceBarrier(1,
		&CD3DX12_RESOURCE_BARRIER::Transition(mBackBuffers[mCurrBackBuffer].Get(),
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

	cameraPassBuffer->Unmap(0,nullptr);
	cameraPassBuffer->Release();

	mRootSignatures.clear();
	mPsos.clear();

	mBackBufferDSVhHeap->Release();
	mBackBufferRTVHeap->Release();

	for (size_t i = 0; i != Graphic_mBackBufferNum;i++) {
		mBackBuffers[i] = nullptr;
	}
	mDepthStencilBuffer->Release();

	mCommandQueue->Release();
	mDrawCmdList->Release();
	mDrawCmdAlloc->Release();

	mFence->Release();
	mDxgiFactory->Release();
	mDxgiSwapChain->Release();
	mDevice->Release();
}

void Graphic::BindShader(Shader* shader) {
	if (state != BEGIN_COMMAND_RECORDING) return;
	ID3D12PipelineState* pipelineState = mPsos[shader->name].Get();
	ID3D12RootSignature* rootSig = mRootSignatures[shader->rootSignatureName].Get();

	mDrawCmdList->SetPipelineState(pipelineState);
	mDrawCmdList->SetGraphicsRootSignature(rootSig);
}

void Graphic::BindConstantBuffer(ID3D12Resource* res, size_t slot) {
	if (state != BEGIN_COMMAND_RECORDING) return;
	
	mDrawCmdList->SetGraphicsRootConstantBufferView(slot,res->GetGPUVirtualAddress());
}

void Graphic::BindMainCameraPass(size_t slot) {
	if (state == BEGIN_COMMAND_RECORDING) {
		cameraPassBufferData->cameraPos = mainCamera.getPosition();
		cameraPassBufferData->view = mainCamera.getViewMat().T();
		cameraPassBufferData->perspect = mainCamera.getPerspectMat().T();

		if (slot > 16) { return; }

		mDrawCmdList->SetGraphicsRootConstantBufferView(1, cameraPassBuffer->GetGPUVirtualAddress());
	}
}

void Graphic::Draw(D3D12_VERTEX_BUFFER_VIEW* vbv, size_t start, size_t num, D3D_PRIMITIVE_TOPOLOGY topolgy) {
	if (state != BEGIN_COMMAND_RECORDING) return;
	
	mDrawCmdList->IASetPrimitiveTopology(topolgy);
	mDrawCmdList->IASetVertexBuffers(0, 1, vbv);
	mDrawCmdList->DrawInstanced(num, 1, start, 0);
}

void Graphic::Draw(D3D12_VERTEX_BUFFER_VIEW* vbv, D3D12_INDEX_BUFFER_VIEW* ibv, size_t start, size_t num, D3D_PRIMITIVE_TOPOLOGY topolgy) {
	if (state != BEGIN_COMMAND_RECORDING) return;

	mDrawCmdList->IASetPrimitiveTopology(topolgy);

	mDrawCmdList->IASetIndexBuffer(ibv);
	mDrawCmdList->IASetVertexBuffers(0, 1, vbv);
	mDrawCmdList->DrawIndexedInstanced(num, 1, start, 0, 0);
}