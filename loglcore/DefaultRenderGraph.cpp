#include "DefaultRenderGraph.h"
#include "graphic.h"



namespace RenderGraph {

	bool DefaultRenderGraph::BuildRenderGraph() {
		size_t screenWidth = gGraphic.GetScreenWidth(), screenHeight = gGraphic.GetScreenHeight();

		
		auto depthBuffer = ClaimResourceNodeDS("depth buffer", screenWidth, screenHeight);
		depthBuffer->CreateDsvDescriptor(AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_DSV));

		auto gbuffer0 = ClaimResourceNodeRT("gbuffer0",  screenWidth, screenHeight,
			TEXTURE_FORMAT_HALF4,Game::Vector4());
		gbuffer0->CreateRtvDescriptor(AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));

		auto gbuffer1 = ClaimResourceNodeRT("gbuffer1", screenWidth, screenHeight,
			TEXTURE_FORMAT_HALF4, Game::Vector4());
		gbuffer1->CreateRtvDescriptor(AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));

		auto gbuffer2 = ClaimResourceNodeRT("gbuffer2", screenWidth, screenHeight,
			TEXTURE_FORMAT_HALF4, Game::Vector4());
		gbuffer2->CreateRtvDescriptor(AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));

		auto colorBuffer = ClaimResourceNodeRT("color buffer", screenWidth, screenHeight,
			TEXTURE_FORMAT_HALF4, Game::Vector4());
		colorBuffer->CreateRtvDescriptor(AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));
		


		if(!RenderGraph::BuildRenderGraph()){
			return false;
		}
		return true;
	}

}