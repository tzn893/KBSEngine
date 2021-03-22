#include "RenderGraphScheduler.h"
#include <set>
#include <queue>

namespace RenderGraph {
	bool RenderGraphScheduler::Build(std::vector<PassNode*>& nodeList,
		const std::vector<std::vector<size_t>>& dependency,
		RenderGraphContext* context) {
		//we assume that one texture will only be written once
		if (dependency.size() != nodeList.size()) return false;
		std::unordered_map<ResourceNodeID, RenderGraphNode*> outputMap;
		std::vector<RenderGraphNode*> renderGraphNodeList;

		for (auto n : nodeList) {
			auto newNode = std::make_unique<RenderGraphNode>(n,nodes.size());
			renderGraphNodeList.push_back(newNode.get());
			nodes.emplace_back(std::move(newNode));
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

			for (auto i : dependency[ni]) {
				if (i < nodeList.size()) 
					nodes[ni]->AddDependency(renderGraphNodeList[i]);
			}
		}

		
		if (!CompileOrder()) {
			OUTPUT_DEBUG_STRING("RenderGraphScheduler::Build : fail to compile the graph,there are cycles of "
				"resources' dependencies in the graph\n");
			return false;
		}

		for (auto n : order) {
			if (!n->node->Initialize(context))
				return false;
		}

		return true;
	}

	bool RenderGraphScheduler::CompileOrder() {
		std::vector<std::vector<bool>> dependTable(nodes.size(),std::vector<bool>(nodes.size(), false));

		for (auto& n : nodes) {
			for (auto c : n->childs) {
				dependTable[n->nodeID][c->nodeID] = true;
			}
		}

		std::queue<RenderGraphNode*> next;
		for (auto r : roots) next.push(r);

		while (!next.empty()) {
			RenderGraphNode* node = next.front();
			next.pop();
			for (auto c : node->childs) {
				dependTable[node->nodeID][c->nodeID] = false;
				bool flag = true;
				for (auto d : c->dependency) {
					if (dependTable[d->nodeID][c->nodeID]) {
						flag = false;
						break;
					}
				}
				if (flag) {
					next.push(c);
				}
			}
			order.push_back(node);
		}

		//if the number of sorted nodes doesn't match the input nodes,there must be some cycles in the graph
		//we return a false value to indicate that the graph is invalid and the compilation fails
		if (order.size() != nodes.size()) {
			return false;
		}

		return true;
	}


	//we will excute the render graph in order
	//maybe we can use AsycExcute in the future
	bool RenderGraphScheduler::Excute(RenderGraphContext* context) {
		for (auto n : order) {
			n->node->Excute(context);
		}
		return true;
	}

	void RenderGraphScheduler::Finalize(RenderGraphContext* context) {
		for (auto& n : nodes) {
			n->node->Finalize(context);
		}
	}
}