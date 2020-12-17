#pragma once

#include "RenderPass.h"
#include "d3dcommon.h"

#include "ConstantBuffer.h"
#include "ShaderDataStruct.h"
#include "DescriptorAllocator.h"
#include "Texture.h"

using PhongObjectID = size_t;

struct PhongMaterialTexture {
	Texture* diffuseMap;
	Texture* normalMap;
};

class PhongRenderPass : public RenderPass {
public:
	virtual size_t GetPriority() override { return 5; }

	virtual bool   Initialize(UploadBatch* batch = nullptr) override;
	virtual void   Render(Graphic* graphic, RENDER_PASS_LAYER layer) override ;

	virtual void   finalize() override;

	void		   BindLightData(LightData* data,size_t offset = 0,size_t num = 1);
	void		   BindAmbientLightData(Game::Vector3 color);
	
	PhongObjectID  AllocateObjectPass();
	void		   DeallocateObjectPass(PhongObjectID& objId);

	ObjectPass*    GetObjectPass(PhongObjectID id);

	void		   DrawObject(D3D12_VERTEX_BUFFER_VIEW* vbv,size_t start,size_t num,
							PhongObjectID id,PhongMaterialTexture* tex = nullptr);
	void		   DrawObject(D3D12_VERTEX_BUFFER_VIEW* vbv,D3D12_INDEX_BUFFER_VIEW* ibv,
							size_t start,size_t num,PhongObjectID id,PhongMaterialTexture* tex = nullptr);
private:
	std::wstring psoName = L"Phong";
	std::wstring rootSigName = L"Phong";

	std::wstring texPsoName = L"PhongTex";
	std::wstring texRootSigName = L"PhongTex";

	std::unique_ptr<ConstantBuffer<LightPass>> lightPass;
	std::unique_ptr<ConstantBuffer<ObjectPass>> objectPass;
	size_t objBufferSize = 256;
	std::vector<PhongObjectID>  avaliableObjectBuffers;
	size_t allocatedBufferNum = 0;

	struct ObjectElement {
		D3D12_VERTEX_BUFFER_VIEW* vbv;
		D3D12_INDEX_BUFFER_VIEW * ibv;

		size_t start;
		size_t num;
		PhongObjectID objectID;

		D3D12_GPU_DESCRIPTOR_HANDLE diffuseMap;
		D3D12_GPU_DESCRIPTOR_HANDLE normalMap;
	};

	std::vector<ObjectElement> objQueue;
	std::vector<ObjectElement> objTexQueue;
	std::unique_ptr<DescriptorHeap> mHeap;

	struct ShadowLightPass {
		Game::Mat4x4 lightView;
	};
	Game::Mat4x4 orthoMat;
	size_t depthWidth = 1024, depthHeight = 1024;
	D3D12_RECT mDepthRect;
	D3D12_VIEWPORT mDepthView;
	std::unique_ptr<Texture> mDepthRTVTex,mDepthDSVTex;
	std::unique_ptr<ConstantBuffer<ShadowLightPass>> mDepthLightView;
	static constexpr size_t mainLightIndex = 0;
	std::wstring shadowPsoName = L"shadowPsoName";
	std::wstring shadowRootSigName = L"shadowRootSig";
	float shadowDistance = 20.;

	void UpdateShadowLightView();
};