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

static ComPtr<ID3D12GraphicsCommandList> mipCmdList;
static ComPtr<ID3D12CommandAllocator>    mipCmdAlloc;
static ComPtr<ID3D12Fence>				 mFence;
static size_t							 mFenceValue = 0;
static HANDLE							 mEvent;
static std::unique_ptr<DescriptorHeap>   heap;


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
		IID_PPV_ARGS(&mipCmdAlloc));
	device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, mipCmdAlloc.Get(),
		nullptr, IID_PPV_ARGS(&mipCmdList));
	
	mipCmdList->Close();

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

	mipCmdAlloc->Reset();
	mipCmdList->Reset(mipCmdAlloc.Get(), mipPSO.Get());
	mipCmdList->SetComputeRootSignature(mipRootSig.Get());

	ID3D12DescriptorHeap* heaps[] = {heap->GetHeap()};
	mipCmdList->SetDescriptorHeaps(1, heaps);
	mipCmdList->SetComputeRootDescriptorTable(4, heap->UploadDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
		srcHandle->cpuHandle).gpuHandle);
	if(initState != D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
		mipCmdList->ResourceBarrier(1,
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
			mipCmdList->SetComputeRootDescriptorTable(i, destDesc);
		}
		
		UINT twidth = Game::imax(width >> offset, 1),
			theight = Game::imax(height >> offset, 1);
		Game::Vector2 texelSize = Game::Vector2(1. / (float)twidth, 1. / (float)theight);

		GenerateMipmapState* state = GMipmapState->GetBufferPtr(sIndex);
		state->SrcMip = srcMip;
		state->TarMipNum = numMip;
		state->TexelSize = texelSize;
		state->TextureSize[0] = twidth, state->TextureSize[1] = theight;

		mipCmdList->SetComputeRootConstantBufferView(5,GMipmapState->GetADDR(sIndex));

		size_t dispatchX = (twidth + 7) / 8 , dispatchY = (theight + 7) / 8;
		mipCmdList->Dispatch(dispatchX, dispatchY, 1);

		offset += numMip;
		sIndex++;
	}

	if (initState != D3D12_RESOURCE_STATE_UNORDERED_ACCESS) {
		mipCmdList->ResourceBarrier(1,
			&CD3DX12_RESOURCE_BARRIER::Transition(res, 
				D3D12_RESOURCE_STATE_UNORDERED_ACCESS, 
				initState)
		);
	}

	mipCmdList->Close();

	ID3D12CommandQueue* cmdQueue = gGraphic.GetCommandQueue();
	ID3D12CommandList* toExcute[] = { mipCmdList.Get() };
	cmdQueue->ExecuteCommandLists(_countof(toExcute), toExcute);

	mFenceValue++;
	cmdQueue->Signal(mFence.Get(), mFenceValue);
	if (mFence->GetCompletedValue() < mFenceValue) {
		if (FAILED(mFence->SetEventOnCompletion(mFenceValue, mEvent))) {
			OUTPUT_DEBUG_STRING("fail to set wait event for completion\n");
			exit(-1);
		}
		WaitForSingleObject(mEvent, INFINITE);
	}

	return true;
}