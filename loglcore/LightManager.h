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

class LightSource {
	friend class LightManager;
public:
	void SetLightIntensity(Game::Vector3 i) {
		data.intensity.x = fmax(i.x, 0.);
		data.intensity.y = fmax(i.y, 0.);
		data.intensity.z = fmax(i.z, 0.);
	}
	void SetLightPosition(Game::Vector3 pos) {
		data.position = pos;
	}
	void SetLightDirection(Game::Vector3 dir) {
		data.direction = Game::normalize(dir);
	}

	void SetLightFallStart(float start) {
		start = Game::fmin(start, data.fallEnd);
		data.fallStart = start;
	}

	void SetLightFallEnd(float end) {
		end = Game::fmax(end, data.fallStart);
		data.fallEnd = end;
	}

	void SetLightFallout(float start,float end) {
		end = Game::fmax(start, end);
		data.fallEnd = end;
		data.fallStart = start;
	}

	Game::Vector3 GetLightIntensity() { data.intensity; }
	Game::Vector3 GetLightPosition() { return data.position; }
	Game::Vector3 GetLightDirection() { return data.direction; }

	Game::Vector3 GetLightFallStart() { return data.fallStart; }
	Game::Vector3 GetLightFallEnd() { return data.fallEnd; }

	SHADER_LIGHT_TYPE GetLightType() { return (SHADER_LIGHT_TYPE)data.type; }
	LightSource(size_t index, SHADER_LIGHT_TYPE lightType);
private:
	LightData data;
	size_t    index;
};



class LightManager {
	friend class ShadowRenderPass;
public:
	void Initialize();

	/*LightData GetLightData(LightID index) {
		if (index < SHADER_MAX_LIGHT_STRUCT_NUM) {
			return mLightPass->GetBufferPtr()->lights[index];
		}
		return LightData();
	}*/
	LightSource* GetMainLightData() {
		return lightSources[mainLightIndex].get();
	}
	/*void SetLightData(LightID index,LightData& lp) {
		if (index == mainLightIndex) SetMainLightData(lp);
		mLightPass->GetBufferPtr()->lights[index] = lp;
	}*/
	Game::Vector3 GetAmbientLight() { return mLightPass->GetBufferPtr()->ambient; }
	void SetAmbientLight(Game::Vector3 light) {
		mLightPass->GetBufferPtr()->ambient = Game::Vector4(light,1.);
	}
	void SetMainLightData(LightData& lp) {
		if (lp.type != SHADER_LIGHT_TYPE_DIRECTIONAL) {
			lp.type = SHADER_LIGHT_TYPE_DIRECTIONAL;
			lp.direction = Game::normalize(lp.direction);
		}
		mLightPass->GetBufferPtr()->lights[mainLightIndex] = lp;
		lightSources[mainLightIndex]->data = lp;
	}

	LightSource* AllocateLightSource(SHADER_LIGHT_TYPE type);
	void DeallocateLightSource(LightSource*& lightSource);
	
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

	std::vector<std::unique_ptr<LightSource>> lightSources;
};
inline LightManager gLightManager;