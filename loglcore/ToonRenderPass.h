#pragma once
#include "RenderPass.h"
#include "Texture.h"
#include "ConstantBuffer.h"
#include "ShaderDataStruct.h"

struct ToonPassMaterial {
	Texture* diffuse;
};


using ToonRenderObjectID = size_t;

class ToonRenderPass : public RenderPass {
public:
	virtual size_t GetPriority() override { return 30; }

	virtual bool   Initialize(UploadBatch* batch = nullptr) override;
	virtual void   Render(Graphic* graphic, RENDER_PASS_LAYER layer) override;

	virtual void   finalize() override;

	ToonRenderObjectID AllocateToonROID();
	bool		DeallocateToonROID(ToonRenderObjectID& id);

	ObjectPass* GetObjectPass(ToonRenderObjectID id);
	void		Draw(D3D12_VERTEX_BUFFER_VIEW* vbv, size_t start, size_t num,
		ToonRenderObjectID id, ToonPassMaterial* tex = nullptr) {
		Draw(vbv, nullptr, start, num, id, tex);
	}
	void		Draw(D3D12_VERTEX_BUFFER_VIEW* vbv, D3D12_INDEX_BUFFER_VIEW* ibv,
		size_t start, size_t num, ToonRenderObjectID id, ToonPassMaterial* tex = nullptr);
private:
	std::unique_ptr<ConstantBuffer<ObjectPass>> mObjectPasses;
	std::vector<ToonRenderObjectID> avaliableToonRenderObjects;
	static constexpr size_t objectBufferSize = 1024;
	size_t allocatedObjectNum = 0;
	std::unique_ptr<DescriptorHeap> descriptorHeap;
	static constexpr size_t heapSize = objectBufferSize;

	struct ToonDrawObject {
		D3D12_VERTEX_BUFFER_VIEW* vbv;
		D3D12_INDEX_BUFFER_VIEW * ibv;

		size_t start;
		size_t num;
		ToonRenderObjectID		  objectID;

		D3D12_GPU_DESCRIPTOR_HANDLE diffuse;
	};
	std::vector<ToonDrawObject>   tdoList;
	const wchar_t* toon_render_pass_shading = L"toon_render_pass_shading";
	const wchar_t* toon_render_pass_outline = L"toon_render_pass_outline";

	float		   outlineWidth;
	Game::Vector3  outlineColor;

	Game::Vector3 div;
};