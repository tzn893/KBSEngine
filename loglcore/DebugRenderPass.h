#pragma once
#include "RenderPass.h"
#include "d3dcommon.h"
#include "Mesh.h"
#include "DescriptorAllocator.h"
#include "Shader.h"
#include "Texture.h"
#include "ConstantBuffer.h"

class DebugRenderPass : public RenderPass {
public:
	virtual size_t GetPriority() override { return 50; }

	virtual bool   Initialize(UploadBatch* batch = nullptr) override;
	virtual void   Render(Graphic* graphic, RENDER_PASS_LAYER layer) override;

	virtual void   finalize() override;

	void   DirectDraw(Texture* texture,ID3D12DescriptorHeap* heap = nullptr);
	void   SetViewPort(Game::Vector2 downLeft,Game::Vector2 upRight);

private:
	struct DebugViewPort {
		Game::Vector2 downLeft;
		Game::Vector2 upRight;
	};

	std::unique_ptr<ConstantBuffer<DebugViewPort>> viewPort;
	std::unique_ptr<StaticMesh<Game::Vector4>> mesh;
	Texture* targetTexture;
	ID3D12DescriptorHeap* targetHeap = nullptr;
	bool ddEnabled;
	Shader* shader;
	std::unique_ptr<DescriptorHeap> mHeap;
};