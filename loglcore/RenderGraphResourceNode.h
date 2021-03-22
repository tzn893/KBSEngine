#pragma once
#include "Texture.h"
#include "DescriptorAllocator.h"
#include <functional>
#include <map>

namespace RenderGraph {
	using ResourceNodeID = size_t;

	class ResourceNode {
		friend class ResourceNodeManager;
	public:
		ResourceNode(ResourceNodeID InternalID, const char* name, Texture* texture);

		const char* GetName() const { return name.c_str(); }
		ResourceNodeID GetInternalID() const { return InternalID; }
		Texture*    GetTexture() { return texture; }

		inline void StateTransition(D3D12_RESOURCE_STATES state,TransitionBatch* batch = nullptr) {
			texture->StateTransition(state,batch);
		}

		inline D3D12_RESOURCE_STATES GetResourceState() {
			return texture->GetResourceState();
		}

		//get the srv of the texture.create one if it doesn't exists
		D3D12_CPU_DESCRIPTOR_HANDLE GetSrvDescriptor(D3D12_SHADER_RESOURCE_VIEW_DESC* srvDesc = nullptr);
		//get the uav of the texture.create one if it doesn't exists
		D3D12_CPU_DESCRIPTOR_HANDLE GetUavDescriptor(size_t miplevel = 0,D3D12_UNORDERED_ACCESS_VIEW_DESC* uavDesc = nullptr);

		//create a rtv for the resource node,manually.
		D3D12_CPU_DESCRIPTOR_HANDLE CreateRtvDescriptor(Descriptor desc,D3D12_RENDER_TARGET_VIEW_DESC* rtvDesc = nullptr);
		D3D12_CPU_DESCRIPTOR_HANDLE GetRtvDescriptor();

		//create a dsv for the resource node
		D3D12_CPU_DESCRIPTOR_HANDLE CreateDsvDescriptor(Descriptor desc,D3D12_DEPTH_STENCIL_VIEW_DESC* dsvDesc = nullptr);
		D3D12_CPU_DESCRIPTOR_HANDLE GetDsvDescriptor();

	private:
		Texture* texture;
		ResourceNodeID InternalID;
		std::string name;
	};

	class ResourceNodeManager {
	public:
		template<typename ...Args>
		ResourceNode* Register(const char* name, Args ...tex) {
			Texture* texture = new Texture(tex...);
			if (ResourceNode* rv = DoRegister(texture, name, true); rv != nullptr)
				return rv;
			delete texture;
			return nullptr;
		}

		template<>
		ResourceNode* Register(const char* name, Texture* texture) {
			return DoRegister(texture, name,false);
		}

		bool Destroy(const char* name);
		ResourceNode* FindResourceNode(const char* name);
		ResourceNode* GetResourceNode(size_t internalID);

		void Reset();

	private:
		ResourceNode* DoRegister(Texture* texture,const char* name,bool isConstant);

		struct ResourceItem {
			bool isConstant;
			union {
				std::unique_ptr<Texture> managedResource;
				Texture* registeredResource;
			};
			~ResourceItem();
			ResourceItem(ResourceItem&& other);
			ResourceItem& operator=(ResourceItem&& other);
			ResourceItem() {}

			std::unique_ptr<ResourceNode> node;
		};

		std::map<std::string,ResourceItem>  resources;
		std::vector<ResourceNode*> nodeList;
		std::vector<size_t>        availableIDs;
		size_t internalIDCounter = 0;
	};

}
