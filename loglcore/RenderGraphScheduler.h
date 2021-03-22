#pragma once
#include "RenderGraphPassNode.h"
#include <unordered_set>

namespace RenderGraph {
	


	class RenderGraphScheduler {
	public:
		bool Build(std::vector<PassNode*>& node,
			const std::vector<std::vector<size_t>>& dependency,
			RenderGraphContext* context
		);

		bool Excute(RenderGraphContext* context);

		void Finalize(RenderGraphContext* context);

	private:
		struct RenderGraphNode {
			PassNode* node;
			std::vector<RenderGraphNode*> dependency;
			std::vector<RenderGraphNode*> childs;
			size_t nodeID;

			RenderGraphNode(PassNode* node,size_t nodeID) {
				this->node = node;
				this->nodeID = nodeID;
			}

			void AddDependency(RenderGraphNode* depend) {
				if (depend != nullptr && depend != this &&
					std::find(dependency.begin(),dependency.end(),depend) == dependency.end()) {
					dependency.push_back(depend);
				}
			}
			void AddChild(RenderGraphNode* child) {
				if (child != nullptr && child != this && 
					std::find(childs.begin(),childs.end(),child) == childs.end()) {
					childs.push_back(child);
				}
			}
		};

		bool CompileOrder();

		std::vector<std::unique_ptr<RenderGraphNode>> nodes;
		std::vector<RenderGraphNode*> roots;
		std::vector<RenderGraphNode*> order;
	};
}