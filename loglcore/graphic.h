#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <vector>
#include "d3dcommon.h"

#include "RenderItem.h"
#include "Math.h"
#include "Camera.h"
#include "Shader.h"

#include <filesystem>
#include <map>

constexpr int Graphic_mBackBufferNum = 3;

class Graphic {
public:
	bool initialize(HWND winHnd,size_t width,size_t height);
	
	void begin();
	void end();

	void finalize();
	void onResize(size_t width, size_t height);

	ID3D12Device* GetDevice() {  return mDevice.Get(); }
	ID3D12CommandQueue* GetCommandQueue() { return mCommandQueue.Get(); }

	void BindShader(Shader* shader);
	void BindConstantBuffer(ID3D12Resource* res,size_t slot);
	//if some shader need main camera pass data.they can get it by binding it to any slot
	void BindMainCameraPass(size_t slot = 1);
	void Draw(D3D12_VERTEX_BUFFER_VIEW* vbv,size_t start,size_t num,D3D_PRIMITIVE_TOPOLOGY topolgy = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	void Draw(D3D12_VERTEX_BUFFER_VIEW* vbv,D3D12_INDEX_BUFFER_VIEW* ibv,size_t start,size_t num,D3D_PRIMITIVE_TOPOLOGY topolgy = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	Camera* GetMainCamera() { return &mainCamera; }
private:

	enum GRAPHIC_STATES {
		BEGIN_COMMAND_RECORDING,
		READY
	} state;

	bool createCommandObject();
	bool createSwapChain();
	bool createRTV_DSV();
	bool createShaderAndRootSignatures();

	void FlushCommandQueue();

	std::vector<RenderItem*> renderItems[RENDER_ITEM_LAYER_NUM];
	
	ComPtr<IDXGIFactory>   mDxgiFactory;
	ComPtr<IDXGISwapChain> mDxgiSwapChain;
	ComPtr<ID3D12Device>   mDevice;

	ComPtr<ID3D12DescriptorHeap> mBackBufferRTVHeap;
	size_t mDescriptorHandleSizeRTV;
	ComPtr<ID3D12DescriptorHeap> mBackBufferDSVhHeap;
	size_t mDescriptorHandleSizeDSV;

	size_t mCurrBackBuffer;
	ComPtr<ID3D12Resource> mBackBuffers[Graphic_mBackBufferNum];
	ComPtr<ID3D12Resource> mDepthStencilBuffer;

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
	ComPtr<ID3D12Resource> cameraPassBuffer;
	struct CameraPassBufferData{
		Game::Mat4x4 view;
		Game::Mat4x4 perspect;
		Game::Vector3 cameraPos;
	}* cameraPassBufferData;
};

inline Graphic gGraphic;