#pragma once
#include "d3dcommon.h"
#include "CopyBatch.h"

class Graphic;


enum RENDER_PASS_LAYER {
	RENDER_PASS_LAYER_AFTER_ALL,

	RENDER_PASS_LAYER_TRANSPARENT,
	RENDER_PASS_LAYER_OPAQUE,
	
	RENDER_PASS_LAYER_BEFORE_ALL
};

class RenderPass {
public:
	virtual size_t GetPriority() = 0;

	virtual void   PreProcess() {}
	virtual bool   Initialize(UploadBatch* batch = nullptr) = 0;
	virtual void   Render(Graphic* graphic,RENDER_PASS_LAYER layer) = 0;
	virtual void   PostProcess(ID3D12Resource* renderTarget) {}

	virtual void   finalize() = 0;
};