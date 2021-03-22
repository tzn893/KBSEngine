#include "RenderGraph.h"
#include "graphic.h"

namespace RenderGraph {

	bool RenderGraph::CreatePassNode(
		AnonymousPassNodeHelper::ExcuteCallBack excute,
		const char* name,
		const std::vector<ResourceNode*>& input,
		const std::vector<ResourceNode*>& output,
		const std::vector<std::string> dependencies	) {
		if (hasBuilt) return false;

		if (!passNodeNameTable.count(name)) {
			std::string s = "RenderGraph::CreatePassNode : fail to create pass node " + std::string(name) + " the name has been occuppied\n";
			OUTPUT_DEBUG_STRING(s.c_str());
			return false;
		}

		PassNode* node = AnonymousPassNodeHelper::CreatePassNode(name, GetResourceIDList(input),
			GetResourceIDList(output), excute);
		if (node == nullptr) {
			std::string s = "RenderGraph::CreatePassNode : fail to create pass node " + std::string(name) + "\n";
			OUTPUT_DEBUG_STRING(s.c_str());
			return false;
		}

		passNodes.push_back(node);
		passNodeDependencies.push_back(dependencies);
		passNodeNameTable[name] = passNodes.size() - 1;

		return true;
	}

	bool RenderGraph::CreatePassNode(
		AnonymousPassNodeHelper::InitializeCallBack initialize,
		AnonymousPassNodeHelper::ExcuteCallBack excute,
		AnonymousPassNodeHelper::FinalizeCallBack finalize,
		const char* name,
		const std::vector<ResourceNode*>& input,
		const std::vector<ResourceNode*>& output,
		const std::vector<std::string>& dependencies
	) {
		if (hasBuilt) return false;

		if (!passNodeNameTable.count(name)) {
			std::string s = "RenderGraph::CreatePassNode : fail to create pass node " + std::string(name) + " the name has been occuppied\n";
			OUTPUT_DEBUG_STRING(s.c_str());
			return false;
		}

		PassNode* node = AnonymousPassNodeHelper::CreatePassNode(name, GetResourceIDList(input),
			GetResourceIDList(output),initialize,excute,finalize);
		if (node == nullptr) {
			std::string s = "RenderGraph::CreatePassNode : fail to create pass node " + std::string(name) + "\n";
			OUTPUT_DEBUG_STRING(s.c_str());
			return false;
		}

		passNodes.push_back(node);
		passNodeDependencies.push_back(dependencies);
		passNodeNameTable[name] = passNodes.size() - 1;

		return true;
	}

	bool RenderGraph::CreatePassNode(
		PassNode* node,
		const char* name,
		const std::vector<ResourceNode*>& input,
		const std::vector<ResourceNode*>& output,
		const std::vector<std::string>& dependencies
	) {
		if (hasBuilt) return false;

		if (!passNodeNameTable.count(name)) {
			std::string s = "RenderGraph::CreatePassNode : fail to create pass node " + std::string(name) + " the name has been occuppied\n";
			OUTPUT_DEBUG_STRING(s.c_str());
			return false;
		}

		if (node == nullptr) {
			std::string s = "RenderGraph::CreatePassNode : fail to create pass node " + std::string(name) + " you should not pass a nullptr value to the nodes\n";
			OUTPUT_DEBUG_STRING(s.c_str());
			return false;
		}

		passNodes.push_back(node);
		passNodeDependencies.push_back(dependencies);
		passNodeNameTable[name] = passNodes.size() - 1;

		return true;
	}

	bool RenderGraph::BuildRenderGraph() {
		if (hasBuilt) return false;

		context.graphic = &gGraphic;
		context.resourceNodeManager = &resourceNodeManager;


		std::vector<std::vector<size_t>> dependencyTable;
		for (auto& d : passNodeDependencies) {
			std::vector<size_t> table;
			for (const auto& dn : d) {
				auto item = passNodeNameTable.find(dn);
				if (item == passNodeNameTable.end()) {
					std::string msg = "RenderGraph::CreatePassNode : Warning the pass node dependency " + dn + " does not exists\n";
					OUTPUT_DEBUG_STRING(msg.c_str());
				}
				else 
					table.push_back(item->second);
			}
			dependencyTable.push_back(table);
		}


		if (!scheduler.Build(passNodes, dependencyTable, &context)) {
			return false;
		}

		passNodes.clear();
		passNodeDependencies.clear();
		passNodeNameTable.clear();
		hasBuilt = true;


		descriptorHeap = std::make_unique<DescriptorHeap>(65536);

		return true;
	}

	void RenderGraph::Excute() {
		if (!hasBuilt) return;

		context.graphic = &gGraphic;
		context.resourceNodeManager = &resourceNodeManager;
		context.descriptorHeap = descriptorHeap.get();

		scheduler.Excute(&context);
	}

	void RenderGraph::FinalizeRenderGraph() {

	}

	std::vector<size_t> RenderGraph::GetResourceIDList(const std::vector<ResourceNode*>& nid) {
		std::vector<size_t> ids;
		for (auto n : nid) {
			ids.push_back(n->GetInternalID());
		}
		return ids;
	}
}