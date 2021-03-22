#pragma once
#include "graphic.h"
#include "RenderGraphResourceNode.h"
#include "RenderGraphScheduler.h"

namespace RenderGraph {
	
	class RenderGraph {
	public:
		template<typename ...Args>
		ResourceNode* ClaimResourceNode(const char* name, Args ...args) {
			return resourceNodeManager.Register(name, args...);
		}

		ResourceNode* ClaimResourceNodeRT( const char* name, size_t width, size_t height,
			TEXTURE_FORMAT format, Game::Vector4 clearColor) {
			D3D12_CLEAR_VALUE cv;
			cv.Color[0] = clearColor[0];
			cv.Color[1] = clearColor[1];
			cv.Color[2] = clearColor[2];
			cv.Color[3] = clearColor[3];
			cv.Format = Texture::GetFormat(format);

			return resourceNodeManager.Register(name, width, height, format, TEXTURE_FLAG_ALLOW_RENDER_TARGET,
				D3D12_RESOURCE_STATE_COMMON, &cv);
		}

		ResourceNode* ClaimResourceNodeDS(const char* name,size_t width, size_t height) {
			D3D12_CLEAR_VALUE cv;
			cv.DepthStencil.Depth = 1.f;
			cv.DepthStencil.Stencil = 0.f;
			cv.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

			std::make_unique<Texture>(width, height, TEXTURE_FORMAT_DEPTH_STENCIL, TEXTURE_FLAG_ALLOW_DEPTH_STENCIL,
				D3D12_RESOURCE_STATE_COMMON, &cv);

			return resourceNodeManager.Register(name, width, height, TEXTURE_FORMAT_DEPTH_STENCIL, TEXTURE_FLAG_ALLOW_DEPTH_STENCIL,
				D3D12_RESOURCE_STATE_COMMON, &cv);
		}


		ResourceNode* ClaimResourceNode(const char* name, Texture* tex) {
			return resourceNodeManager.Register(name, tex);
		}

		ResourceNode* EvalueResourceNode(const char* name,Texture* tex) {
			return resourceNodeManager.Register(name, tex);
		}

		bool CreatePassNode(
			AnonymousPassNodeHelper::ExcuteCallBack excute,
			const char* name,
			const std::vector<ResourceNode*>& input,
			const std::vector<ResourceNode*>& output,
			const std::vector<std::string> dependencies = {}
		);

		bool CreatePassNode(
			AnonymousPassNodeHelper::InitializeCallBack initialize,
			AnonymousPassNodeHelper::ExcuteCallBack excute,
			AnonymousPassNodeHelper::FinalizeCallBack finalize,
			const char* name,			
			const std::vector<ResourceNode*>& input,
			const std::vector<ResourceNode*>& output,
			const std::vector<std::string>& dependencies = {}
		);

		bool CreatePassNode(
			PassNode* node,
			const char* name,
			const std::vector<ResourceNode*>& input,
			const std::vector<ResourceNode*>& output,
			const std::vector<std::string>& dependencies = {}
		);

		Descriptor AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE type, size_t num = 1) {
			return descriptorHeap->Allocate(num, type);
		}

		virtual bool BuildRenderGraph();

		virtual void Excute();

		void FinalizeRenderGraph();

	private:
		
		std::vector<size_t> GetResourceIDList(const std::vector<ResourceNode*>& nid);

		ResourceNodeManager resourceNodeManager;
		RenderGraphScheduler scheduler;
		std::unique_ptr<DescriptorHeap> descriptorHeap;
		std::vector<PassNode*> passNodes;
		std::vector<std::vector<std::string>> passNodeDependencies;
		std::map<std::string, size_t> passNodeNameTable;

		RenderGraphContext context;
		bool hasBuilt = false;
	};

}