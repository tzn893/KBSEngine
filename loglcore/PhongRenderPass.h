#pragma once

#include "RenderPass.h"
#include "d3dcommon.h"

#include "ConstantBuffer.h"
#include "ShaderDataStruct.h"

using PhongObjectID = size_t;

class PhongRenderPass : public RenderPass {
public:
	virtual size_t GetPriority() override { return 5; }

	virtual bool   Initialize(UploadBatch* batch = nullptr) override;
	virtual void   Render(Graphic* graphic, RENDER_PASS_LAYER layer) override ;

	virtual void   finalize() override;

	void		   BindLightData(LightData* data,size_t offset = 0,size_t num = 1);
	void		   BindAmbientLightData(Game::Vector3 color);
	
	PhongObjectID  AllocateObjectPass();
	void		   DeallocateObjectPass(PhongObjectID objId);

	ObjectPass*    GetObjectPass(PhongObjectID id);

	void		   DrawObject(D3D12_VERTEX_BUFFER_VIEW* vbv,size_t start,size_t num,
							PhongObjectID id);
	void		   DrawObject(D3D12_VERTEX_BUFFER_VIEW* vbv,D3D12_INDEX_BUFFER_VIEW* ibv,
							size_t start,size_t num,PhongObjectID id);
private:
	std::wstring psoName = L"Phong";
	std::wstring rootSigName = L"Phong";

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
	};

	std::vector<ObjectElement> objQueue;
};