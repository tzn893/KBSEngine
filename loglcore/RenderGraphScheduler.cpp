#include "RenderGraphScheduler.h"



namespace RenderGraph {
	bool RenderGraphScheduler::Build(std::vector<PassNode*>& nodeList) {
		//we assume that one texture will only be written once
		std::unordered_map<ResourceNodeID, RenderGraphNode*> outputMap;

		for (auto n : nodeList) {
			nodes.emplace_back(std::make_unique<RenderGraphNode>(n));
			for (auto o : n->output) {
				//one texture will only be written once
				if (outputMap.count(o)) return false;
				outputMap[o] = nodes.rbegin()->get();
			}
		}

		for (int ni = 0; ni != nodes.size();ni++) {
			for (auto i : nodeList[ni]->input) {
				if (!outputMap.count(i)) return false;
				nodes[ni]->AddDependency(outputMap[i]);
				outputMap[i]->AddChild(nodes[ni].get());
			}
			if (nodes[ni]->dependency.empty()) 
				roots.push_back(nodes[ni].get());
		}

		std::unordered_set<RenderGraphNode*> visited;

		
		for (int i = 0; i != roots.size();i++) {
			if (!CheckForLoops(visited,roots[i])) {
				return false;
			}
		}

		for (auto node : nodeList) {
			if (!node->Initialize()) {
				return false;
			}
		}

		return true;
	}

	bool RenderGraphScheduler::CheckForLoops(std::unordered_set<RenderGraphNode*>& visited,RenderGraphScheduler::RenderGraphNode* root) {
		if (visited.count(root)) {
			return false;
		}
		visited.insert(root);

		for (auto c : root->childs) {
			if (!CheckForLoops(visited, c)) return false;
		}
		return true;
	}

	void RenderGraphScheduler::ResetRenderGraph(RenderGraphNode* node) {
		if (node == nullptr) return;
		node->dpCounter = node->dependency.size();
		for (auto n : node->childs) {
			ResetRenderGraph(n);
		}
	}

	bool RenderGraphScheduler::Excute() {
		for (auto r : roots) {
			ResetRenderGraph(r);
		}

		std::vector<RenderGraphNode*> waiting = roots;
		while (!waiting.empty()) {
			RenderGraphNode* next = nullptr;
			for (auto iter = waiting.begin(); iter != waiting.end();iter++) {
				if ((*iter)->dpCounter != 0) continue;
				next = *iter;
				waiting.erase(iter);
				break;
			}
			if (next == nullptr) return false;

			next->node->Excute();
			

		}
	}

	RenderGraphScheduler::~RenderGraphScheduler() {
		for (auto& n : nodes) {
			n->node->Finalize();
		}
	}
}