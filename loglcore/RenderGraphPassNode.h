#pragma once

#include <vector>
#include "RenderGraphResourceNode.h"
#include <functional>

class Graphic;

namespace RenderGraph {
	

	class RenderGraphContext {
		friend class RenderGraph;
	public:
		ResourceNode* GetResourceNode(ResourceNodeID id) { return resourceNodeManager->GetResourceNode(id); }
		Graphic*      GetGraphic() { return graphic; }

		Descriptor UploadDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle) { 
			return descriptorHeap->UploadDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,cpuHandle,1); 
		}

	private:
		ResourceNodeManager* resourceNodeManager;
		Graphic*			 graphic;
		DescriptorHeap*		 descriptorHeap;
	};


	class PassNode {
		friend class RenderGraphScheduler;
	public:
		PassNode(const char* name,const std::vector<ResourceNodeID>& input,
			const std::vector<ResourceNodeID>& output) :
			input(input), output(output), name(name) {}

		const char* GetName() { return name.c_str(); }

		virtual void Excute(RenderGraphContext* context) = 0;
		virtual bool Initialize(RenderGraphContext* context) = 0;
		virtual void Finalize(RenderGraphContext* context) = 0;

		virtual ~PassNode() {}

	protected:


		std::vector<ResourceNodeID> input;
		std::vector<ResourceNodeID> output;
		std::string name;
	};


	class AnonymousPassNodeHelper {
	public:
		using InitializeCallBack = std::function<bool(RenderGraphContext*)>;
		using ExcuteCallBack = std::function<void(RenderGraphContext*)>;
		using FinalizeCallBack = std::function<void(RenderGraphContext*)>;

		class AnonymousPassNode : public PassNode {
		public:
			AnonymousPassNode(const char* name,const std::vector<ResourceNodeID>& input,
				const std::vector<ResourceNodeID>& output,
				InitializeCallBack initialize,
				ExcuteCallBack excute,
				FinalizeCallBack finalize) :PassNode(name, input, output),
				excute(excute), initialize(initialize), finalize(finalize) {}

			void Excute(RenderGraphContext* context) override { excute(context); }
			bool Initialize(RenderGraphContext* context) override { return initialize(context);}
			void Finalize(RenderGraphContext* context) override { return finalize(context); }
		
		private:
			InitializeCallBack initialize;
			ExcuteCallBack excute;
			FinalizeCallBack finalize;
		};

		static PassNode* CreatePassNode(const char* name,const std::vector<ResourceNodeID>& input,
			const std::vector<ResourceNodeID>& output,
			InitializeCallBack initialize,
			ExcuteCallBack excute,
			FinalizeCallBack finalize);

		static PassNode* CreatePassNode(const char* name,const std::vector<ResourceNodeID>& input,
			const std::vector<ResourceNodeID>& output,
			ExcuteCallBack excute);

		static PassNode* GetPassNode(const char* name);

	private:
		static std::map<std::string, std::unique_ptr<AnonymousPassNode>> nodes;
	}; 


}