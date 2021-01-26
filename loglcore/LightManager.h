#pragma once
#include "ShaderDataStruct.h"
#include "ConstantBuffer.h"

#include "RenderPass.h"
#include "Texture.h"
#include "DescriptorAllocator.h"

constexpr size_t ShadowRenderPassPriority = 200;

class ShadowRenderPass : public RenderPass {
public:
	virtual size_t GetPriority() { return ShadowRenderPassPriority; }

	virtual bool   Initialize(UploadBatch* batch = nullptr) override;
	virtual void   Render(Graphic* graphic, RENDER_PASS_LAYER layer) override;
	
	virtual void   finalize();

	void	 RenderShadowMap(D3D12_VERTEX_BUFFER_VIEW* vbv,D3D12_INDEX_BUFFER_VIEW* ibv,
		size_t start,size_t num,D3D12_GPU_VIRTUAL_ADDRESS cBuffer);
	void	 RenderShadowMap(D3D12_VERTEX_BUFFER_VIEW* vbv,size_t start,
		size_t num,D3D12_GPU_VIRTUAL_ADDRESS cBuffer);

	Texture* GetShadowMap();
	D3D12_GPU_VIRTUAL_ADDRESS GetDepthLightView() { return mDepthLightView->GetADDR(); }

	static Game::Vector2 GetDepthTexelWidth() {
		return Game::Vector2(1.f / (float)depthWidth,1.f / (float)depthHeight);
	}
private:
	void UpdateShadowLightView();

	struct ShadowLightPass {
		Game::Mat4x4 lightView;
	};
	Game::Mat4x4 orthoMat;
	static constexpr size_t depthWidth = 4096, depthHeight = 4096;
	D3D12_RECT mDepthRect;
	D3D12_VIEWPORT mDepthView;
	std::unique_ptr<Texture> mDepthRTVTex, mDepthDSVTex;
	std::unique_ptr<ConstantBuffer<ShadowLightPass>> mDepthLightView;
	static constexpr size_t mainLightIndex = 0;
	std::wstring shadowPsoName = L"shadowPsoName";
	std::wstring shadowRootSigName = L"shadowRootSig";
	float shadowDistance = 20.;

	std::unique_ptr<DescriptorHeap> mHeap;

	struct ObjectElement {
		D3D12_VERTEX_BUFFER_VIEW* vbv;
		D3D12_INDEX_BUFFER_VIEW * ibv;

		size_t start;
		size_t num;
		D3D12_GPU_VIRTUAL_ADDRESS cBuffer;
	};
	std::vector<ObjectElement> objQueue;
};

//render pass for rendering emission objects
class EmissionRenderPass : public RenderPass {

};

class LightManager {
	friend class ShadowRenderPass;
public:
	void Initialize();

	LightData GetLightData(size_t index) {
		if (index < SHADER_MAX_LIGHT_STRUCT_NUM) {
			return mLightPass->GetBufferPtr()->lights[index];
		}
		return LightData();
	}
	LightData GetMainLightData() {
		return mLightPass->GetBufferPtr()->lights[mainLightIndex];
	}
	void SetLightData(size_t index,LightData& lp) {
		if (index == mainLightIndex) SetMainLightData(lp);
		mLightPass->GetBufferPtr()->lights[index] = lp;
	}
	void SetAmbientLight(Game::Vector3 light) {
		mLightPass->GetBufferPtr()->ambient = Game::Vector4(light,1.);
	}
	void SetMainLightData(LightData& lp) {
		if (lp.type != SHADER_LIGHT_TYPE_DIRECTIONAL) {
			lp.type = SHADER_LIGHT_TYPE_DIRECTIONAL;
			lp.direction = Game::normalize(lp.direction);
		}
		mLightPass->GetBufferPtr()->lights[mainLightIndex] = lp;
	}

	void BindLightPass2ConstantBuffer(size_t slot);
	bool EnableShadowRenderPass();

	ShadowRenderPass* GetShadowRenderPass() {
		if (shadowEnabled) {
			return mShadowRenderPass.get();
		}
		return nullptr;
	}
private:
	std::unique_ptr<ConstantBuffer<LightPass>> mLightPass;
	static constexpr size_t mainLightIndex = 0;

	std::unique_ptr<ShadowRenderPass> mShadowRenderPass;
	bool shadowEnabled = false;
};
inline LightManager gLightManager;