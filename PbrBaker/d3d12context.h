#pragma once
#include <d3d12.h>
#include <d3dcompiler.h>
#include <dxgi1_4.h>

#include <map>
#include <string>

#include <wrl.h>
#include <functional>

#include <Windows.h>

#include "..\loglcore\d3d12x.h"
#include <vector>

template<typename T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

class D3D12Context {
public:
	using ICallBack = std::function<bool(D3D12Context*)>;
	using UCallBack = std::function<bool(D3D12Context*)>;

	bool Initialize(ICallBack callback,HWND wind);
	bool Update(UCallBack callback);
	void Finailize();

	ComPtr<ID3DBlob> CompileShader(const wchar_t* filename,const char* target,const char* entry);
	bool			 CreateRootSignature(const std::vector<CD3DX12_ROOT_PARAMETER>& parameters,
					const std::vector<CD3DX12_STATIC_SAMPLER_DESC>& ssd,
					const char* name);
	bool		     CreatePipelineState(D3D12_GRAPHICS_PIPELINE_STATE_DESC gpsd,const char* name);

	D3D12_CPU_DESCRIPTOR_HANDLE GetSrvCpuDescriptor(size_t offset) {
			size_t hsize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			CD3DX12_CPU_DESCRIPTOR_HANDLE res(srvHeap->GetCPUDescriptorHandleForHeapStart());
			return res.Offset(offset, hsize);
	}

	D3D12_CPU_DESCRIPTOR_HANDLE GetRtvDescriptor(size_t offset) {
		size_t hsize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		CD3DX12_CPU_DESCRIPTOR_HANDLE res(rtvHeap->GetCPUDescriptorHandleForHeapStart());
		return res.Offset(offset, hsize);

	}



	D3D12_GPU_DESCRIPTOR_HANDLE GetGpuDescriptor(size_t offset) {
		size_t hsize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		CD3DX12_GPU_DESCRIPTOR_HANDLE res(srvHeap->GetGPUDescriptorHandleForHeapStart());
		return res.Offset(offset, hsize);
	}

	std::map<std::string, ComPtr<ID3D12Resource>>      resources;
	std::map<std::string, ComPtr<ID3D12PipelineState>> psos;
	std::map<std::string, ComPtr<ID3D12RootSignature>> rootSigs;

	ComPtr<ID3D12DescriptorHeap> rtvHeap;
	ComPtr<ID3D12DescriptorHeap> srvHeap;

	ComPtr<ID3D12Device> device;
	ComPtr<IDXGIFactory> factory;
	ComPtr<IDXGISwapChain> swapChain;

	ComPtr<ID3D12GraphicsCommandList> cmdList;
	ComPtr<ID3D12CommandQueue> cmdQueue;
	ComPtr<ID3D12CommandAllocator> cmdAlloc;

	HANDLE eve;
	ComPtr<ID3D12Fence> fence;
	UINT64 fenceVal = 0;
};