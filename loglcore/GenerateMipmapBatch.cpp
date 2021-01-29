#include "GenerateMipmapBatch.h"
#include "graphic.h"

static const wchar_t* psoName = L"generate_mip_map";
static const wchar_t* rootSigName = L"generate_mip_map";

static ComPtr<ID3D12RootSignature> mipRootSig;
static ComPtr<ID3D12PipelineState> mipPSO;

struct GenerateMipmapState {
	uint32_t SrcMip;
	uint32_t TarMipNum;
	Game::Vector2 TexelSize;
	uint32_t TextureSize[2];
};
static std::unique_ptr<ConstantBuffer<GenerateMipmapState>> GMipmapState;
static bool initialized = false;

static ComPtr<ID3D12GraphicsCommandList> cmdList;
static ComPtr<ID3D12CommandAllocator>    cmdAlloc;
static ComPtr<ID3D12Fence>				 mFence;
static size_t							 mFenceValue = 0;
static HANDLE							 mEvent;
static std::unique_ptr<DescriptorHeap>   heap;

static ComPtr<ID3D12RootSignature>		 irrRootSig;
static ComPtr<ID3D12PipelineState>		 irrPso;

const wchar_t*	irrRootSigName = L"gen_irr";
const wchar_t*  irrPsoName	   = L"gen_irr";
static bool irr_gen_initialized = false;

struct GenIrrProj{
	Game::Mat4x4 mright;
	Game::Mat4x4 mleft;
	Game::Mat4x4 mup;
	Game::Mat4x4 mdown;
	Game::Mat4x4 mfront;
	Game::Mat4x4 mback;
};
static std::unique_ptr<ConstantBuffer<GenIrrProj>> mGenIrrProj;

bool GenerateMipmapBatch::initialize() {
	Game::RootSignature root(6, 1);
	root[0].initAsDescriptorTable(0, 0, 1, D3D12_DESCRIPTOR_RANGE_TYPE_UAV);
	root[1].initAsDescriptorTable(1, 0, 1, D3D12_DESCRIPTOR_RANGE_TYPE_UAV);
	root[2].initAsDescriptorTable(2, 0, 1, D3D12_DESCRIPTOR_RANGE_TYPE_UAV);
	root[3].initAsDescriptorTable(3, 0, 1, D3D12_DESCRIPTOR_RANGE_TYPE_UAV);
	root[4].initAsDescriptorTable(0, 0, 1, D3D12_DESCRIPTOR_RANGE_TYPE_SRV);
	root[5].initAsConstantBuffer(0, 0);

	CD3DX12_STATIC_SAMPLER_DESC ssDesc(0);
	ssDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	ssDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	ssDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;

	ssDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	root.InitializeSampler(0, ssDesc);
	if (!gGraphic.CreateRootSignature(rootSigName, &root)) {
		OUTPUT_DEBUG_STRING("fail to create root signature for mipmap generation\n");
		return false;
	}
	mipRootSig = root.GetRootSignature();

	ComputeShader* cs = gShaderManager.loadComputeShader(L"../shader/GenerateMip.hlsl",
		"GenerateMipmap", rootSigName, rootSigName, nullptr);
	if (cs == nullptr) {
		OUTPUT_DEBUG_STRING("fail to create compute shader for mipmap generating\n");
		return false;
	}

	Game::ComputePSO cPso = Game::ComputePSO::Default();
	if (!gGraphic.CreateComputePipelineStateObject(cs,&cPso,psoName)) {
		OUTPUT_DEBUG_STRING("fail to create compute pso for mipmap generating\n");
		return false;
	}
	mipPSO = cPso.GetPSO();

	ID3D12Device* device = gGraphic.GetDevice();
	heap = std::make_unique<DescriptorHeap>(40);

	HRESULT hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(&cmdAlloc));
	device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, cmdAlloc.Get(),
		nullptr, IID_PPV_ARGS(&cmdList));
	
	cmdList->Close();

	mEvent = CreateEventEx(NULL, NULL, NULL, EVENT_ALL_ACCESS);
	if (mEvent == NULL) {
		OUTPUT_DEBUG_STRING("ERROR : fail to create fence event for generating mipmap\n");
		return false;
	}
	hr = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mFence));
	if (FAILED(hr)) {
		OUTPUT_DEBUG_STRING("ERROR : fail to create fence for generating mipmap\n");
		return false;
	}

	GMipmapState = std::make_unique<ConstantBuffer<GenerateMipmapState>>(gGraphic.GetDevice(),10);

	return true;
}

#include "GraphicUtil.h"

static void FlushCommandQueue(ID3D12CommandQueue* cmdQueue) {
	mFenceValue++;
	cmdQueue->Signal(mFence.Get(), mFenceValue);
	if (mFence->GetCompletedValue() < mFenceValue) {
		if (FAILED(mFence->SetEventOnCompletion(mFenceValue, mEvent))) {
			OUTPUT_DEBUG_STRING("fail to set wait event for completion\n");
			exit(-1);
		}
		WaitForSingleObject(mEvent, INFINITE);
	}
}

bool GenerateMipmapBatch::Generate(ID3D12Resource* res,D3D12_RESOURCE_STATES initState
	,size_t width,size_t height,
	size_t mipnum,Descriptor* destHandles,
	Descriptor* srcHandle) {
	if (!initialized) {
		if (!initialize()) {
			return false;
		}
		initialized = true;
	}

	ID3D12CommandQueue* queue = gGraphic.GetCommandQueue();

	heap->ClearUploadedDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	cmdAlloc->Reset();
	cmdList->Reset(cmdAlloc.Get(), mipPSO.Get());
	cmdList->SetComputeRootSignature(mipRootSig.Get());

	ID3D12DescriptorHeap* heaps[] = {heap->GetHeap()};
	cmdList->SetDescriptorHeaps(1, heaps);
	cmdList->SetComputeRootDescriptorTable(4, heap->UploadDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
		srcHandle->cpuHandle).gpuHandle);
	if(initState != D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
		cmdList->ResourceBarrier(1,
			&CD3DX12_RESOURCE_BARRIER::Transition(res, initState,
				D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
		);


	UINT offset = 1,sIndex = 0;
	while (offset < mipnum) {
		UINT srcMip = offset - 1;
		UINT numMip = Game::imin(4,mipnum - offset);
		D3D12_GPU_DESCRIPTOR_HANDLE destDesc;
		for (size_t i = 0; i != numMip;i++) {
			destDesc = heap->UploadDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, destHandles[i + offset].cpuHandle).gpuHandle;
			cmdList->SetComputeRootDescriptorTable(i, destDesc);
		}
		
		UINT twidth = Game::imax(width >> offset, 1),
			theight = Game::imax(height >> offset, 1);
		Game::Vector2 texelSize = Game::Vector2(1. / (float)twidth, 1. / (float)theight);

		GenerateMipmapState* state = GMipmapState->GetBufferPtr(sIndex);
		state->SrcMip = srcMip;
		state->TarMipNum = numMip;
		state->TexelSize = texelSize;
		state->TextureSize[0] = twidth, state->TextureSize[1] = theight;

		cmdList->SetComputeRootConstantBufferView(5,GMipmapState->GetADDR(sIndex));

		size_t dispatchX = (twidth + 7) / 8 , dispatchY = (theight + 7) / 8;
		cmdList->Dispatch(dispatchX, dispatchY, 1);

		offset += numMip;
		sIndex++;
	}

	if (initState != D3D12_RESOURCE_STATE_UNORDERED_ACCESS) {
		cmdList->ResourceBarrier(1,
			&CD3DX12_RESOURCE_BARRIER::Transition(res, 
				D3D12_RESOURCE_STATE_UNORDERED_ACCESS, 
				initState)
		);
	}

	cmdList->Close();

	ID3D12CommandQueue* cmdQueue = gGraphic.GetCommandQueue();
	ID3D12CommandList* toExcute[] = { cmdList.Get() };
	cmdQueue->ExecuteCommandLists(_countof(toExcute), toExcute);

	/*mFenceValue++;
	cmdQueue->Signal(mFence.Get(), mFenceValue);
	if (mFence->GetCompletedValue() < mFenceValue) {
		if (FAILED(mFence->SetEventOnCompletion(mFenceValue, mEvent))) {
			OUTPUT_DEBUG_STRING("fail to set wait event for completion\n");
			exit(-1);
		}
		WaitForSingleObject(mEvent, INFINITE);
	}*/
	FlushCommandQueue(cmdQueue);

	return true;
}

static bool init_gen_irr(DXGI_FORMAT targetFormat) {
	Game::RootSignature rootSig(2, 1);
	rootSig[0].initAsConstantBuffer(0, 0);
	rootSig[1].initAsDescriptorTable(0, 0, 1, D3D12_DESCRIPTOR_RANGE_TYPE_SRV);
	rootSig.InitializeSampler(0, CD3DX12_STATIC_SAMPLER_DESC(0));

	if (!gGraphic.CreateRootSignature(irrRootSigName,&rootSig)) {
		OUTPUT_DEBUG_STRING("fail to create root signature for irradience generation\n");
		return false;
	}

	irrRootSig = rootSig.GetRootSignature();

	std::vector<D3D12_INPUT_ELEMENT_DESC> layout{};
	Shader* shader = gShaderManager.loadShader(L"../shader/PBRIrradianceBaking.hlsl", "VS", "PS",
		irrRootSigName, layout, irrPsoName);
	if (shader == nullptr) {
		OUTPUT_DEBUG_STRING("fail to create shader for irradience generation\n");
		return false;
	}

	Game::GraphicPSO mPso;
	mPso.LazyBlendDepthRasterizeDefault();
	CD3DX12_DEPTH_STENCIL_DESC dsd(D3D12_DEFAULT);
	dsd.DepthEnable = false;
	dsd.StencilEnable = false;
	mPso.SetDepthStencilState(dsd);
	CD3DX12_RASTERIZER_DESC rsd(D3D12_DEFAULT);
	rsd.CullMode = D3D12_CULL_MODE_NONE;
	mPso.SetRasterizerState(rsd);
	mPso.SetSampleMask(UINT_MAX);
	mPso.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
	mPso.SetFlag(D3D12_PIPELINE_STATE_FLAG_NONE);
	DXGI_FORMAT rtvFormats[6] = { targetFormat ,targetFormat ,targetFormat ,
		targetFormat ,targetFormat , targetFormat};
	mPso.SetRenderTargetFormat(6, rtvFormats);
	
	if (!gGraphic.CreatePipelineStateObject(shader,&mPso,irrPsoName)) {
		OUTPUT_DEBUG_STRING("fail to create pso for irradince generation\n");
		return false;
	}

	irrPso = mPso.GetPSO();
	
	mGenIrrProj = std::make_unique<ConstantBuffer<GenIrrProj>>(gGraphic.GetDevice());

	GenIrrProj* irr = mGenIrrProj->GetBufferPtr();
	irr->mright = Game::Mat4x4(
					0, 0, 0, 1,
					0, 1, 0, 0,
				   -1, 0, 0, 0,
					0, 0, 0, 0).T();
	irr->mleft  = Game::Mat4x4(
					 0, 0, 0,-1,
					 0, 1, 0, 0,
					 1, 0, 0, 0,
					 0,	0, 0, 0).T();
	irr->mfront = Game::Mat4x4(
					 1, 0, 0, 0,
					 0, 1, 0, 0,
					 0, 0, 0, 1,
					 0, 0, 0, 0).T();
	irr->mback  = Game::Mat4x4(
					-1, 0, 0, 0,
					 0, 1, 0, 0,
					 0, 0, 0,-1,
					 0, 0, 0, 0).T();
	irr->mup    = Game::Mat4x4(
					-1, 0, 0, 0,
					 0, 0, 0, 1,
					 0, 1, 0, 0,
					 0, 0, 0, 0).T();
	irr->mdown = Game::Mat4x4(
					 1, 0, 0, 0,
					 0, 0, 0,-1,
					 0, 1, 0, 0,
					 0, 0, 0, 0).T();
	return true;
}

bool GenerateMipmapBatch::GenerateIBLIrradience(ID3D12Resource* source, 
	ID3D12Resource* target,
	Descriptor srcSrvHandle,
	D3D12_RESOURCE_STATES tarInitState) {
	if (!initialized) {
		if (!initialize()) return false;
		initialized = true;
	}
	if (!irr_gen_initialized) {
		if (!init_gen_irr(target->GetDesc().Format)) return false;
		irr_gen_initialized = true;
	}

	ID3D12CommandQueue* queue = gGraphic.GetCommandQueue();

	heap->ClearUploadedDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	heap->ClearAllocatedDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	cmdAlloc->Reset();
	cmdList->Reset(cmdAlloc.Get(), irrPso.Get());
	cmdList->SetGraphicsRootSignature(irrRootSig.Get());
	

	if (tarInitState != D3D12_RESOURCE_STATE_RENDER_TARGET) {
		cmdList->ResourceBarrier(1,
			&CD3DX12_RESOURCE_BARRIER::Transition(target,tarInitState,
				D3D12_RESOURCE_STATE_RENDER_TARGET));
	}
	size_t tarHeight = target->GetDesc().Height,
		tarWidth = target->GetDesc().Width;
	D3D12_RECT sissor;
	sissor.bottom = tarHeight;
	sissor.top = 0;
	sissor.left = 0;
	sissor.right = tarWidth;

	D3D12_VIEWPORT viewPort;
	viewPort.Width = tarWidth;
	viewPort.Height = tarHeight;
	viewPort.TopLeftX = 0;
	viewPort.TopLeftY = 0;
	viewPort.MinDepth = 0;
	viewPort.MaxDepth = 1.0f;
	cmdList->RSSetScissorRects(1, &sissor);
	cmdList->RSSetViewports(1, &viewPort);

	D3D12_CPU_DESCRIPTOR_HANDLE rtvs[6];
	for (size_t i = 0; i != 6;i++) {
		Descriptor desc = heap->Allocate(1, D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
		rtvDesc.Format = target->GetDesc().Format;
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
		rtvDesc.Texture2DArray.MipSlice = 0;
		rtvDesc.Texture2DArray.PlaneSlice = 0;
		rtvDesc.Texture2DArray.FirstArraySlice = i;
		rtvDesc.Texture2DArray.ArraySize = 1;
		gGraphic.GetDevice()->CreateRenderTargetView(target, &rtvDesc, desc.cpuHandle);
		rtvs[i] = desc.cpuHandle;
	}
	cmdList->OMSetRenderTargets(6, rtvs, false, nullptr);

	ID3D12DescriptorHeap* heaps[] = {heap->GetHeap()};
	cmdList->SetDescriptorHeaps(_countof(heaps), heaps);

	cmdList->SetGraphicsRootDescriptorTable(1, heap->UploadDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
		srcSrvHandle.cpuHandle).gpuHandle);
	cmdList->SetGraphicsRootConstantBufferView(0, mGenIrrProj->GetADDR());
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	cmdList->DrawInstanced(6, 1, 0, 0);
	
	if (tarInitState != D3D12_RESOURCE_STATE_RENDER_TARGET) {
		cmdList->ResourceBarrier(1,
			&CD3DX12_RESOURCE_BARRIER::Transition(target,
				D3D12_RESOURCE_STATE_RENDER_TARGET, tarInitState)
		);
	}

	cmdList->Close();

	ID3D12CommandQueue* cmdQueue = gGraphic.GetCommandQueue();
	ID3D12CommandList* toExcute[] = { cmdList.Get() };
	cmdQueue->ExecuteCommandLists(_countof(toExcute), toExcute);

	FlushCommandQueue(cmdQueue);
	return true;
}