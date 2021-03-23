#pragma once

#include "RenderGraph.h"


namespace RenderGraph {
	struct DefaultRenderGraphTextures {
		Texture* diffuse = nullptr;
		Texture* normal = nullptr;
		Texture* metallic = nullptr;
		Texture* roughness = nullptr;
		Texture* emission = nullptr;
	};


	class DefaultRenderGraph : public RenderGraph {
	public:
		bool BuildRenderGraph() override;

		void RenderByDefaultRenderGraph(
			D3D12_VERTEX_BUFFER_VIEW* vbv,
			D3D12_INDEX_BUFFER_VIEW* ibv,
			size_t start,
			size_t num,

			Game::Mat4x4 world,
			Material     mat,
			DefaultRenderGraphTextures textures
		);

	private:
		struct ObjectElement {
			D3D12_VERTEX_BUFFER_VIEW* vbv;
			D3D12_INDEX_BUFFER_VIEW * ibv;

			size_t start;
			size_t num;

			D3D12_GPU_DESCRIPTOR_HANDLE diffuseMap;
			D3D12_GPU_DESCRIPTOR_HANDLE normalMap;
			//D3D12_GPU_DESCRIPTOR_HANDLE specularMap;
			D3D12_GPU_DESCRIPTOR_HANDLE metallicMap;
			D3D12_GPU_DESCRIPTOR_HANDLE roughnessMap;
			D3D12_GPU_DESCRIPTOR_HANDLE emissionMap;
		};
		std::vector<ObjectElement> objs;

		std::unique_ptr<ConstantBuffer<ObjectPass>> objectPassPool;

	};

}