#pragma once
#include "RenderPass.h"
#include "ConstantBuffer.h"
#include "d3dcommon.h"
#include "Texture.h"
#include "Math.h"

#include "DescriptorAllocator.h"
#include "Shader.h"


struct SpriteData {
	Game::Vector3 Position;
	float rotation;
	Game::Vector2 Scale;
	Game::Vector4 Color;
};

typedef size_t SpriteID;

class SpriteRenderPass : public RenderPass {
public:
	virtual size_t GetPriority() override { return 10; }

	virtual bool   Initialize(UploadBatch* batch)  override;
	virtual void   Render(Graphic* graphic,RENDER_PASS_LAYER layer)      override;
	virtual void   finalize()    override;
	virtual void   PostProcess(ID3D12Resource* renderTarget) override;
	

	SpriteID RegisterTexture(Texture* tex,D3D12_SHADER_RESOURCE_VIEW_DESC* srv = nullptr);
	//all the sprites's color's alpha value will be ignored on this pipeline
	void     DrawSprite(size_t num,SpriteData* data,SpriteID sprite);
	void     DrawSpriteTransparent(size_t num, SpriteData* data, SpriteID sprite);

	inline void SetViewCenter(float y, float x = -1.) {
		if (x < 0) {
			viewCenter.x = viewCenter.x / viewCenter.y * y;
		}
		viewCenter.y = y;
		UpdateViewConstant();
	}

	inline void	SetViewScale(const Game::Vector2& vec) { SetViewScale(vec.x,vec.y); }
	inline void SetViewScale(float x, float y) {
		viewScale.x = x, viewScale.y = y;
		UpdateViewConstant();
	}
private:
	void     UpdateViewConstant();
	struct SpriteGroup {
		//Texture* texture;
		D3D12_GPU_DESCRIPTOR_HANDLE sprite;
		size_t   startInstance;
		size_t   instanceNum;
	};

	SpriteGroup AllocateGroupConstant(size_t num, SpriteData* data, SpriteID sprite,bool adjustAlpha);

	struct SpriteInstanceConstant {
		Game::Vector4 color;
		Game::Mat4x4 world;
	};

	struct SpriteViewConstant {
		Game::Mat4x4 view;
	};

	struct RegisteredTexture {
		Texture* tex;
		D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;
	};

	size_t instanceBufferSize, allocatedBufferSize;
	std::unique_ptr<ConstantBuffer<SpriteInstanceConstant>> spriteInstances;
	
	std::unique_ptr<ConstantBuffer<SpriteViewConstant>> spriteViewConstant;
	Game::Vector2 viewCenter;
	Game::Vector2 viewScale;

	std::unique_ptr<DescriptorHeap>  heap;
	std::vector<RegisteredTexture> registedTextures;

	std::vector<SpriteGroup> renderGroupOpaque;
	std::vector<SpriteGroup> renderGroupTransparent;
	
	Shader* spriteShader;

	ComPtr<ID3D12Resource> mVBuffer;
	D3D12_VERTEX_BUFFER_VIEW mVbv;
};