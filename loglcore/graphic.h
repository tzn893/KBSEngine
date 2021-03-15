#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <vector>
#include <map>
#include <queue>

#include "d3dcommon.h"

#include "Math.h"
#include "Camera.h"
#include "Shader.h"
#include "RootSignature.h"
#include "PipelineStateObject.h"

#include "SpriteRenderPass.h"
#include "PhongRenderPass.h"
#include "ShaderDataStruct.h"
#include "DebugRenderPass.h"
#include "SkyboxRenderPass.h"
#include "PostProcessRenderPass.h"
#include "DeferredRenderPass.h"
#include "ToonRenderPass.h"

#include "TransitionBatch.h"
#include "GraphicUtil.h"

constexpr int Graphic_mBackBufferNum = 3;

class Graphic {
	friend class ComputeCommand;
	friend class TransitionBatch;
public:
	bool initialize(HWND winHnd, size_t width, size_t height);

	void begin();
	void end();

	void finalize();
	void onResize(size_t width, size_t height);

	ID3D12Device* GetDevice() { return mDevice.Get(); }
	ID3D12CommandQueue* GetCommandQueue() { return mCommandQueue.Get(); }

	void BindShader(Shader* shader);
	bool BindPSOAndRootSignature(const wchar_t* psoName, const wchar_t* rootSignatureName);

	void BindConstantBuffer(ID3D12Resource* res, size_t slot, size_t offset = 0);
	void BindShaderResource(ID3D12Resource* res, size_t slot, size_t offset = 0);
	void BindConstantBuffer(D3D12_GPU_VIRTUAL_ADDRESS vaddr, size_t slot);
	void BindShaderResource(D3D12_GPU_VIRTUAL_ADDRESS vaddr, size_t slot);
	
	template<typename T>
	void Bind32bitConstant(size_t slot,T value, size_t offset = 0) {
		static_assert(!std::is_pointer<T>::value, "value will be passed by value rather than reference");
		if constexpr (sizeof(T) <= 4) {
			mDrawCmdList->SetGraphicsRoot32BitConstant(slot, Pack32bitNum(value), offset);
		}
		else {
			mDrawCmdList->SetGraphicsRoot32BitConstants(slot, sizeof(T) / 4, &value, offset);
		}
	}

	//if some shader need main camera pass data.they can get it by binding it to any slot
	void BindMainCameraPass(size_t slot = 1);
	void BindDescriptorHandle(D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle, size_t slot);
	void BindDescriptorHeap(ID3D12DescriptorHeap* const * heap, size_t num = 1);
	void BindDescriptorHeap(ID3D12DescriptorHeap* heap);

	void Draw(D3D12_VERTEX_BUFFER_VIEW* vbv, size_t start, size_t num, D3D_PRIMITIVE_TOPOLOGY topolgy = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	void Draw(D3D12_VERTEX_BUFFER_VIEW* vbv, D3D12_INDEX_BUFFER_VIEW* ibv, size_t start, size_t num, D3D_PRIMITIVE_TOPOLOGY topolgy = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	void DrawInstance(D3D12_VERTEX_BUFFER_VIEW* vbv, size_t start, size_t num, size_t instanceNum, D3D_PRIMITIVE_TOPOLOGY topolgy = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	void DrawInstance(D3D12_VERTEX_BUFFER_VIEW* vbv, D3D12_INDEX_BUFFER_VIEW* ibv, size_t start, size_t num, size_t instanceNum, D3D_PRIMITIVE_TOPOLOGY topolgy = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	bool CreateRootSignature(std::wstring name, Game::RootSignature* rootSig);

	bool CreatePipelineStateObjectRP(Shader* shader, Game::GraphicPSORP* pso, const wchar_t* name = nullptr) {
		return CreatePipelineStateObject(shader, pso, name, true);
	}
	bool CreatePipelineStateObject(Shader* shader, Game::GraphicPSO* pso, const wchar_t* name = nullptr) {
		return CreatePipelineStateObject(shader, pso, name, false);
	}
	bool CreateComputePipelineStateObject(ComputeShader* shader, Game::ComputePSO* pso, const wchar_t* name = nullptr);

	void ResourceTransition(ID3D12Resource* resource, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after);
	void ResourceTransition(ID3D12Resource** resources, D3D12_RESOURCE_STATES* before, D3D12_RESOURCE_STATES* after,
		size_t num);

	void ResourceCopy(ID3D12Resource* Dest, ID3D12Resource* Source);
	void ResourceCopy(ID3D12Resource* Dest, ID3D12Resource* Source, D3D12_RESOURCE_STATES destInitState,
		D3D12_RESOURCE_STATES sourceInitState, D3D12_RESOURCE_STATES destAfterState,
		D3D12_RESOURCE_STATES sourceAfterState);

	void BindCurrentBackBufferAsRenderTarget(bool clear = false, float* clearValue = nullptr);
	void BindRenderTarget(D3D12_CPU_DESCRIPTOR_HANDLE* cpuHandle, D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle, size_t rtvNum = 1,
		bool clear = false, float* clearValule = nullptr, D3D12_VIEWPORT* viewPort = nullptr, D3D12_RECT* rect = nullptr,
		bool clearDepth = false);

	Camera* GetMainCamera() { return &mainCamera; }
	float   GetHeightWidthRatio() { return (float)mWinWidth / (float)mWinHeight; }

	size_t  GetScreenHeight() { return mWinHeight; }
	size_t  GetScreenWidth() { return mWinWidth; }

	template<typename RPType>
	RPType* GetRenderPass() {
		if constexpr (std::is_same<RPType, SpriteRenderPass>::value) {
			return spriteRenderPass.get();
		}
		else if constexpr (std::is_same<RPType, PhongRenderPass>::value) {
			if (phongRenderPass.get() == nullptr && deferredRenderPass.get() == nullptr) {
				phongRenderPass = std::make_unique<PhongRenderPass>();
				RenderPass* rps[] = { phongRenderPass.get() };
				RegisterRenderPasses(rps);
			}
			return phongRenderPass.get();
		}
		else if constexpr (std::is_same<RPType, DebugRenderPass>::value) {
			return debugRenderPass.get();
		}
		else if constexpr (std::is_same<RPType, SkyboxRenderPass>::value) {
			return skyboxRenderPass.get();
		}
		else if constexpr (std::is_same<RPType, DeferredRenderPass>::value) {
			if (phongRenderPass.get() == nullptr && deferredRenderPass.get() == nullptr) {
				deferredRenderPass = std::make_unique<DeferredRenderPass>();
				RegisterRenderPass(deferredRenderPass.get());
			}
			return deferredRenderPass.get();
		}
		else  if constexpr (std::is_same<RPType,PostProcessRenderPass>::value) {
			return postProcessRenderPass.get();
		}
		else if constexpr (std::is_same<RPType,ToonRenderPass>::value) {
			return toonRenderPass.get();
		}
		else{
			return nullptr;
		}
	}

	template<typename RPType>
	void DisableRenderPass() {
		if constexpr (std::is_same<RPType, SpriteRenderPass>::value) {
			FindRPAndErase(spriteRenderPass.get());
			spriteRenderPass->finalize();
			spriteRenderPass.release();
		}
		else if constexpr (std::is_same<RPType, PhongRenderPass>::value) {
			if (phongRenderPass.get() == nullptr)
				return;
			FindRPAndErase(phongRenderPass.get());
			phongRenderPass->finalize();
			phongRenderPass.release();
		}
		else if constexpr (std::is_same<RPType, DebugRenderPass>::value) {
			FindRPAndErase(debugRenderPass.get());
			debugRenderPass->finalize();
			debugRenderPass.release();
		}
		else if constexpr (std::is_same<RPType, SkyboxRenderPass>::value) {
			FindRPAndErase(skyboxRenderPass.get());
			skyboxRenderPass->finalize();
			skyboxRenderPass.release();
		}
		else if constexpr (std::is_same<RPType, DeferredRenderPass>::value) {
			FindRPAndErase(deferredRenderPass.get());
			deferredRenderPass->finalize();
			deferredRenderPass.release();
		}
		else if constexpr (std::is_same<RPType,ToonRenderPass>::value) {
			FindRPAndErase(toonRenderPass.get());
			toonRenderPass->finalize();
			toonRenderPass.release();
		}
		else {
			return;//otherwise we do nothing
		}
	}

	bool    RegisterRenderPasses(RenderPass** RP, size_t num = 1);
	inline  bool RegisterRenderPass(RenderPass* rp) {
		RenderPass* rps[] = { rp };
		return RegisterRenderPasses(rps);
	}

	void    SetSissorRect(D3D12_RECT* sissorRect, size_t num = 1);
	void	SetViewPort(D3D12_VIEWPORT* vp, size_t num = 1);

	void	RestoreOriginalViewPortAndRect();

	D3D12_RECT GetDefaultSissorRect() { return sissorRect; }
	D3D12_VIEWPORT GetDefaultViewPort() { return viewPort; }

	void	SetDefaultClearColor(float* newColor) { memcpy(mRTVClearColor, newColor, sizeof(float) * 4); }

	inline size_t GetDescriptorHandleSize(D3D12_DESCRIPTOR_HEAP_TYPE type) {
		if (type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) {
			return mDescriptorHandleSizeCBVSRVUAV;
		}
		else if (type == D3D12_DESCRIPTOR_HEAP_TYPE_DSV) {
			return mDescriptorHandleSizeDSV;
		}
		else if (type == D3D12_DESCRIPTOR_HEAP_TYPE_RTV) {
			return mDescriptorHandleSizeRTV;
		}

		return 0;
	}

	inline	DXGI_FORMAT	 GetRenderTargetFormat() { return mRenderTargetFormat; }
	inline  DXGI_FORMAT	 GetDepthStencilFormat() { return mBackBufferDepthFormat; }

	inline  void		 SetHDRExposure(float exposure) { this->exposure = exposure; }

	void	UpdateCameraPass();
	
	float   GetExposure() { return exposure; }
private:
	void FindRPAndErase(RenderPass* rp);
	bool CreatePipelineStateObject(Shader* shader, Game::GraphicPSO* pso, const wchar_t* name, bool rp);

	enum GRAPHIC_STATES {
		BEGIN_COMMAND_RECORDING,
		READY
	} state;

	bool createCommandObject();
	bool createSwapChain();
	bool createRTV_DSV();
	bool createShaderAndRootSignatures();
	bool createDevice();

	bool createRenderPasses();

	void FlushCommandQueue();

	ComPtr<IDXGIFactory>   mDxgiFactory;
	ComPtr<IDXGISwapChain> mDxgiSwapChain;
	ComPtr<ID3D12Device>   mDevice;

	ComPtr<ID3D12DescriptorHeap> mBackBufferRTVHeap;
	size_t mDescriptorHandleSizeRTV;
	ComPtr<ID3D12DescriptorHeap> mBackBufferDSVhHeap;
	size_t mDescriptorHandleSizeDSV;

	size_t mDescriptorHandleSizeCBVSRVUAV;

	size_t mCurrBackBuffer;
	ComPtr<ID3D12Resource> mBackBuffers[Graphic_mBackBufferNum];
	ComPtr<ID3D12Resource> mRenderTargets[Graphic_mBackBufferNum];
	ComPtr<ID3D12Resource> mDepthStencilBuffer;

	ComPtr<ID3D12DescriptorHeap> mSRVHeap;
	float exposure = 1.;

	DXGI_FORMAT mRenderTargetFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;
	DXGI_FORMAT mBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT mBackBufferDepthFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	ComPtr<ID3D12CommandQueue> mCommandQueue;

	ComPtr<ID3D12GraphicsCommandList> mDrawCmdList;
	ComPtr<ID3D12CommandAllocator> mDrawCmdAlloc;

	Game::Color mClearColor;
	size_t mWinWidth, mWinHeight;
	bool useMass = false;
	size_t massQuality;

	HWND winHandle;
	

	uint32_t fenceValue;
	ComPtr<ID3D12Fence> mFence;
	HANDLE fenceEvent;

	D3D12_RECT sissorRect;
	D3D12_VIEWPORT viewPort;

	float mRTVClearColor[4] = {.2,.2,.5,1.};

	std::map<std::wstring, ComPtr<ID3D12RootSignature>> mRootSignatures;
	std::map<std::wstring, ComPtr<ID3D12PipelineState>> mPsos;

	Camera mainCamera;
	std::unique_ptr<ConstantBuffer<CameraPass>> cameraPassData;

	//map is a BST.So we can use it as a iterable priority queue
	std::map<size_t, std::vector<RenderPass*>> RPQueue;
	std::unique_ptr<SpriteRenderPass> spriteRenderPass;
	std::unique_ptr<PhongRenderPass>  phongRenderPass;
	std::unique_ptr<DebugRenderPass>  debugRenderPass;
	std::unique_ptr<SkyboxRenderPass> skyboxRenderPass;
	std::unique_ptr<PostProcessRenderPass> postProcessRenderPass;
	std::unique_ptr<DeferredRenderPass> deferredRenderPass;
	std::unique_ptr<ToonRenderPass>   toonRenderPass;

	std::vector<D3D12_RESOURCE_BARRIER> barriers;
};

inline Graphic gGraphic;