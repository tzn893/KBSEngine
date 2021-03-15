#pragma once
#include "RenderGraphPassNode.h"
#include <unordered_set>

namespace RenderGraph {
	class RenderGraphScheduler {
	public:
		bool Build(std::vector<PassNode*>& node);

		bool Excute();

		~RenderGraphScheduler();

	private:
		struct RenderGraphNode {
			PassNode* node;
			std::vector<RenderGraphNode*> dependency;
			std::vector<RenderGraphNode*> childs;
			size_t dpCounter;

			RenderGraphNode(PassNode* node) {
				this->node = node;
				dpCounter = 0;
			}

			void AddDependency(RenderGraphNode* depend) {
				if (depend != nullptr) {
					dependency.push_back(depend);
				}
			}
			void AddChild(RenderGraphNode* child) {
				if (child != nullptr && child != this) {
					childs.push_back(child);
				}
			}
		};

		bool CheckForLoops(std::unordered_set<RenderGraphNode*>& visited,RenderGraphNode* node);
		void ResetRenderGraph(RenderGraphNode* root);

		std::vector<std::unique_ptr<RenderGraphNode>> nodes;
		std::vector<RenderGraphNode*> roots;
	};
}