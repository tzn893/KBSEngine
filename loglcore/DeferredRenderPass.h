#pragma once
#include "RenderPass.h"

#include "Texture.h"
#include "ConstantBuffer.h"
#include "ShaderDataStruct.h"
#include "Mesh.h"

using DeferredRenderPassID = int64_t;


struct DeferredRenderPassTexture {
	Texture* normal;
	Texture* diffuse;
	//Texture* specular;
	Texture* metallic;
	Texture* roughness;
	Texture* emission;
};

class DeferredRenderPass : public RenderPass {
public:
	virtual size_t GetPriority() { return 10; }

	virtual void   PreProcess() {}
	virtual bool   Initialize(UploadBatch* batch) override;
	virtual void   Render(Graphic* graphic, RENDER_PASS_LAYER layer) override;
	virtual void   PostProcess(ID3D12Resource* renderTarget) {}

	virtual void   finalize() override;

	DeferredRenderPassID  AllocateObjectPass();
	void		   DeallocateObjectPass(DeferredRenderPassID& objId);

	ObjectPass*    GetObjectPass(DeferredRenderPassID id);

	void		   DrawObject(D3D12_VERTEX_BUFFER_VIEW* vbv, size_t start, size_t num,
		DeferredRenderPassID id, DeferredRenderPassTexture* tex = nullptr) {
		DrawObject(vbv, nullptr, start, num, id, tex);
	}
	void		   DrawObject(D3D12_VERTEX_BUFFER_VIEW* vbv, D3D12_INDEX_BUFFER_VIEW* ibv,
		size_t start, size_t num, DeferredRenderPassID id, DeferredRenderPassTexture* tex = nullptr);

	void		   DrawSkinnedObject(D3D12_VERTEX_BUFFER_VIEW* vbv,D3D12_INDEX_BUFFER_VIEW* ibv,
		size_t start,size_t num,DeferredRenderPassID id,DeferredRenderPassTexture* tex,
		D3D12_GPU_VIRTUAL_ADDRESS boneTransformBuffer);

	D3D12_CPU_DESCRIPTOR_HANDLE GetGBufferHandle(size_t offset) {
		if (offset < GBufferNum) return GBuffer[offset]->GetShaderResourceViewCPU();
		return { 0 };
	}
private:
	static constexpr size_t GBufferNum = 4;
	std::unique_ptr<Texture> GBuffer[GBufferNum];

	struct ObjectElement {
		D3D12_VERTEX_BUFFER_VIEW* vbv;
		D3D12_INDEX_BUFFER_VIEW * ibv;

		size_t start;
		size_t num;
		DeferredRenderPassID objectID;

		D3D12_GPU_DESCRIPTOR_HANDLE diffuseMap;
		D3D12_GPU_DESCRIPTOR_HANDLE normalMap;
		//D3D12_GPU_DESCRIPTOR_HANDLE specularMap;
		D3D12_GPU_DESCRIPTOR_HANDLE metallicMap;
		D3D12_GPU_DESCRIPTOR_HANDLE roughnessMap;
		D3D12_GPU_DESCRIPTOR_HANDLE emissionMap;
	};
	std::vector<ObjectElement> objQueue;

	struct AnimatedObjectElement {
		ObjectElement objEle;
		D3D12_GPU_VIRTUAL_ADDRESS boneTransform;
	};
	std::vector<AnimatedObjectElement> animatedObjQueue;

	std::unique_ptr<ConstantBuffer<ObjectPass>> objConstants;
	std::vector<size_t> availableConstantBuffers;
	size_t allocatedConstantBuffers;
	static constexpr size_t defaultConstantBufferSize = 1024;

	static constexpr size_t defaultDescriptorHeapSize = defaultConstantBufferSize * 5;
	std::unique_ptr<DescriptorHeap> descriptorHeap;

	const wchar_t* defPreproc = L"deferred_preprocess";
	const wchar_t* defSkinnedPreproc = L"deferred_skinned_preprocess";
	const wchar_t* defShading = L"defferred_shading";
	const wchar_t* lutMapPath = L"../asserts/pbr/BrdfLut.png";

	std::unique_ptr<StaticMesh<Game::Vector4>> mImageVert;
	Texture* lutTex;


	class DeferredShadingPass : public RenderPass {
	public:
		DeferredShadingPass(DeferredRenderPass* drp) :drp(drp) {}

		virtual size_t GetPriority() { return 20; }

		virtual bool   Initialize(UploadBatch* batch = nullptr) { return true; }
		virtual void   Render(Graphic* graphic, RENDER_PASS_LAYER layer) override;

		virtual void   finalize() {};
	private:
		DeferredRenderPass* drp;
	};
	bool activated = false;
	std::unique_ptr<DeferredShadingPass> shadingPass;
};