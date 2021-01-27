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
	Texture* specular;
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

private:
	static constexpr size_t GBufferNum = 3;
	std::unique_ptr<Texture> GBuffer[GBufferNum];

	struct ObjectElement {
		D3D12_VERTEX_BUFFER_VIEW* vbv;
		D3D12_INDEX_BUFFER_VIEW * ibv;

		size_t start;
		size_t num;
		DeferredRenderPassID objectID;

		D3D12_GPU_DESCRIPTOR_HANDLE diffuseMap;
		D3D12_GPU_DESCRIPTOR_HANDLE normalMap;
		D3D12_GPU_DESCRIPTOR_HANDLE specularMap;
	};
	std::vector<ObjectElement> objQueue;

	std::unique_ptr<ConstantBuffer<ObjectPass>> objConstants;
	std::vector<size_t> availableConstantBuffers;
	size_t allocatedConstantBuffers;
	static constexpr size_t defaultConstantBufferSize = 1024;

	static constexpr size_t defaultDescriptorHeapSize = defaultConstantBufferSize * 3;
	std::unique_ptr<DescriptorHeap> descriptorHeap;

	const wchar_t* defPreproc = L"deferred_preprocess";
	const wchar_t* defShading = L"defferred_shading";

	std::unique_ptr<StaticMesh<Game::Vector4>> mImageVert;
};