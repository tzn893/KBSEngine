#pragma once

#include <vector>
#include "RenderGraphResourceNode.h"


namespace RenderGraph {
	class PassNode {
		friend class RenderGraphScheduler;
	public:
		PassNode(const wchar_t* name, std::vector<ResourceNodeID>& input,
			std::vector<ResourceNodeID>& output) :
			input(input), output(output), name(name){}

		const wchar_t* GetName() { return name.c_str(); }

		virtual void Excute()  = 0;
		virtual bool Initialize() = 0;
		virtual void Finalize() = 0;
		
		virtual ~PassNode() {}
	protected:


		std::vector<ResourceNodeID> input;
		std::vector<ResourceNodeID> output;
		std::wstring name;
	};

	
}