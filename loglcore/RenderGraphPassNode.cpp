#include "RenderGraphPassNode.h"

namespace RenderGraph {
	PassNode* AnonymousPassNodeHelper::GetPassNode(const char* name) {
		if (auto item = nodes.find(name); item != nodes.end()) {
			return item->second.get();
		}
		return nullptr;
	}

	PassNode* AnonymousPassNodeHelper::CreatePassNode(const char* name, const std::vector<ResourceNodeID>& input,
		const std::vector<ResourceNodeID>& output,
		InitializeCallBack initialize,
		ExcuteCallBack excute,
		FinalizeCallBack finalize) {
		if (GetPassNode(name) != nullptr) {
			return nullptr;
		}

		std::unique_ptr<AnonymousPassNode> newNode = std::make_unique<AnonymousPassNode>(name, input, output,
			initialize, excute, finalize);
		PassNode* rv = newNode.get();
		nodes[name] = std::move(newNode);

		return rv;
	}


	PassNode* AnonymousPassNodeHelper::CreatePassNode(const char* name, const std::vector<ResourceNodeID>& input,
		const std::vector<ResourceNodeID>& output,
		ExcuteCallBack excute) {

		return CreatePassNode(name, input, output,
			[](const RenderGraphContext*) {return true; },
			excute,
			[](const RenderGraphContext*) {});
	}
}