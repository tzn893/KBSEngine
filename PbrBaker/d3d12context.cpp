#include "d3d12context.h"

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"d3dcompiler.lib")

static size_t ScoreAdapter(IDXGIAdapter* adapter) {
	DXGI_ADAPTER_DESC adaDesc;
	adapter->GetDesc(&adaDesc);
	//À¬»øÎ¢Èíguna
	if (std::wstring(adaDesc.Description) == L"Microsoft Basic Render Driver") {
		return 0;
	}
	return adaDesc.DedicatedSystemMemory + adaDesc.SharedSystemMemory + adaDesc.DedicatedVideoMemory;
}

bool D3D12Context::Initialize(ICallBack callback, HWND wind) {
	
#ifdef _DEBUG || DEBUG
	ComPtr<ID3D12Debug> debug;
	D3D12GetDebugInterface(IID_PPV_ARGS(&debug));
	debug->EnableDebugLayer();
#endif
	
	HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&factory));
	if (FAILED(hr)) return false;

	ComPtr<IDXGIAdapter> adapter, targetAda = nullptr;
	size_t iadapter = 0, score = 0;
	while (factory->EnumAdapters(iadapter, &adapter) != DXGI_ERROR_NOT_FOUND) {
		size_t mScore = ScoreAdapter(adapter.Get());
		if (mScore > score) {
			targetAda = adapter;
			score = mScore;
		}
		iadapter++;
	}

	if (targetAda == nullptr) {
		hr = D3D12CreateDevice(nullptr,
			D3D_FEATURE_LEVEL_12_0,
			IID_PPV_ARGS(&device));
	}
	else {
		hr = D3D12CreateDevice(targetAda.Get(),
			D3D_FEATURE_LEVEL_12_0,
			IID_PPV_ARGS(&device));
	}

	if (FAILED(hr)) {
		return false;
	}



	{
		D3D12_COMMAND_QUEUE_DESC desc{};
		desc.Flags = D3D12_COMMAND_QUEUE_FLAG_DISABLE_GPU_TIMEOUT;
		desc.NodeMask = 0;
		desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		
		hr = device->CreateCommandQueue(&desc, IID_PPV_ARGS(&cmdQueue));
		if (FAILED(hr)) return false;

		hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&cmdAlloc));
		if (FAILED(hr)) return false;

		hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, cmdAlloc.Get(), nullptr, IID_PPV_ARGS(&cmdList));
		if (FAILED(hr)) return false;

		cmdList->Close();
	}
	
	{
		DXGI_SWAP_CHAIN_DESC scDesc{};
		scDesc.BufferCount = 3;
		scDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		scDesc.BufferDesc.Height = 100;
		scDesc.BufferDesc.Width = 100;
		scDesc.BufferDesc.RefreshRate = { 60,1 };
		scDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		scDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		scDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
		scDesc.OutputWindow = wind;
		scDesc.SampleDesc.Count = 1;
		scDesc.SampleDesc.Quality = 0;
		scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		scDesc.Windowed = true;

		HRESULT hr = factory->CreateSwapChain(cmdQueue.Get(), &scDesc, &swapChain);
		if (FAILED(hr)) return false;
	}

	{
		D3D12_DESCRIPTOR_HEAP_DESC desc;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		desc.NodeMask = 0;
		desc.NumDescriptors = 120;
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

		if (FAILED(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&rtvHeap)))) {
			return false;
		}

		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		desc.NumDescriptors = 40;

		if (FAILED(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&srvHeap)))) {
			return false;
		}
	}

	{
		eve = CreateEventEx(NULL, NULL, NULL, EVENT_ALL_ACCESS);
		if (eve == NULL) return false;

		if (FAILED(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)))) {
			return false;
		}
	}

	
	if (!callback(this)) {
		return false;
	}
}


bool D3D12Context::Update(UCallBack callback) {
	cmdAlloc->Reset();
	cmdList->Reset(cmdAlloc.Get(), nullptr);

	ID3D12DescriptorHeap* heaps[] = {srvHeap.Get()};
	cmdList->SetDescriptorHeaps(1, heaps);

	bool rv = callback(this);

	cmdList->Close();
	
	ID3D12CommandList* list[] = {cmdList.Get()};
	cmdQueue->ExecuteCommandLists(_countof(list), list);

	swapChain->Present(0, 0);

	cmdQueue->Signal(fence.Get(), ++fenceVal);
	if (fence->GetCompletedValue() < fenceVal) {
		if (FAILED(fence->SetEventOnCompletion(fenceVal,eve))) {
			return false;
		}
		WaitForSingleObject(eve, INFINITE);
	}

	return rv;
}

void D3D12Context::Finailize() {

}

#include <iostream>


ComPtr<ID3DBlob> D3D12Context::CompileShader(const wchar_t* filename, const char* target, const char* entry) {

	unsigned int compileFlag = 0;
#ifdef _DEBUG || DEBUG
	compileFlag = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	ComPtr<ID3DBlob> blob,error;
	HRESULT hr = D3DCompileFromFile(filename, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		entry, target, compileFlag, 0, blob.GetAddressOf(), error.GetAddressOf());

	if (FAILED(hr)) {
		std::cout << "fail to compile shader " << filename << " reason : " << reinterpret_cast<char*>(error->GetBufferPointer());
		return nullptr;
	}

	return blob;
}

bool		D3D12Context::CreateRootSignature(const std::vector<CD3DX12_ROOT_PARAMETER>& parameters,
	const std::vector<CD3DX12_STATIC_SAMPLER_DESC>& ssd,
	const char* name) {
	CD3DX12_ROOT_SIGNATURE_DESC rsd(parameters.size(),parameters.data(),
		ssd.size(), ssd.data(), D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> rootSigBlob;
	if (FAILED(D3D12SerializeRootSignature(&rsd,D3D_ROOT_SIGNATURE_VERSION_1_0,rootSigBlob.GetAddressOf(),nullptr))) {
		return false;
	}

	ComPtr<ID3D12RootSignature> rootSig;
	if (FAILED(device->CreateRootSignature(0,rootSigBlob->GetBufferPointer(),rootSigBlob->GetBufferSize(),
		IID_PPV_ARGS(&rootSig)))) {
		return false;
	}

	rootSigs[name] = rootSig;
	return true;

}

bool		D3D12Context::CreatePipelineState(D3D12_GRAPHICS_PIPELINE_STATE_DESC gpsd, const char* name) {
	ComPtr<ID3D12PipelineState> pso;
	if (FAILED(this->device->CreateGraphicsPipelineState(&gpsd,IID_PPV_ARGS(&pso)))) {
		return false;
	}
	psos[name] = pso;
	return true;
}