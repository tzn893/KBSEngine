#include "RenderGraphResourceNode.h"


namespace RenderGraph {

	ResourceNodeManager::ResourceItem::ResourceItem(ResourceItem&& other) {
		if (other.isConstant) {
			std::swap(managedResource, other.managedResource);
		}
		else {
			registeredResource = other.registeredResource;
		}
		std::swap(node, other.node);
		isConstant = other.isConstant;
	}

	ResourceNodeManager::ResourceItem::~ResourceItem() {
		if (isConstant) {
			managedResource.release();
			managedResource = nullptr;
		}
		node.release();
		node = nullptr;
	}


	ResourceNodeManager::ResourceItem& ResourceNodeManager::ResourceItem::operator=(ResourceItem&& other) {
		if (other.isConstant) {
			std::swap(managedResource, other.managedResource);
		}
		else {
			registeredResource = other.registeredResource;
		}
		std::swap(node, other.node);
		isConstant = other.isConstant;
		return *this;
	}


	bool ResourceNodeManager::Destroy(const char* name) {
		if (auto item = resources.find(name); item != resources.end()) {
			size_t internalID = item->second.node->InternalID;
			availableIDs.push_back(internalID);
			nodeList[internalID] = nullptr;
			resources.erase(item);
			return true;
		}
		return false;
	}

	ResourceNode* ResourceNodeManager::FindResourceNode(const char* name) {
		if (auto item = resources.find(name); item != resources.end()) {
			return item->second.node.get();
		}
		return nullptr;
	}

	ResourceNode* ResourceNodeManager::DoRegister(Texture* tex, const char* name, bool isConstant) {
		if (tex == nullptr) return nullptr;
		if (name == nullptr) {
			static size_t _internal_index = 0;
			static std::string buf;
			buf = "unnamed_texture_" + std::to_string(_internal_index++);
			while (FindResourceNode(buf.c_str()) != nullptr) {
				buf = "unnamed_texture_" + std::to_string(_internal_index++);
			}
			name = buf.c_str();
		}
		if (auto item = resources.find(name); item != resources.end()) {
			if (!isConstant && !item->second.isConstant) {
				item->second.registeredResource = tex;
				item->second.node->texture = tex;
				return item->second.node.get();
			}
			return nullptr;
		}

		ResourceItem item;
		item.isConstant = isConstant;
		if (isConstant) {
			item.managedResource = std::unique_ptr<Texture>(tex);
		}
		else {
			item.registeredResource = tex;
		}

		if (!availableIDs.empty()) {
			size_t id = *availableIDs.rbegin();
			availableIDs.pop_back();
			item.node = std::make_unique<ResourceNode>(id, name, tex);
			nodeList[id] = item.node.get();
		}
		else {
			item.node = std::make_unique<ResourceNode>(internalIDCounter++, name, tex);
			nodeList.push_back(item.node.get());
		}
		ResourceNode* rv = item.node.get();
		resources[name] = std::move(item);
		return rv;
	}


	ResourceNode* ResourceNodeManager::GetResourceNode(size_t id) {
		if (id >= nodeList.size()) return nullptr;
		return nodeList[id];
	}

	D3D12_CPU_DESCRIPTOR_HANDLE ResourceNode::GetSrvDescriptor(D3D12_SHADER_RESOURCE_VIEW_DESC* srvDesc) {
		D3D12_CPU_DESCRIPTOR_HANDLE desc = texture->GetShaderResourceViewCPU();
		if (desc.ptr != 0) return desc;
		texture->CreateShaderResourceView(gDescriptorAllocator.AllocateDescriptor(1), srvDesc);
		return texture->GetShaderResourceViewCPU();
	}

	D3D12_CPU_DESCRIPTOR_HANDLE ResourceNode::GetUavDescriptor(size_t miplevel, D3D12_UNORDERED_ACCESS_VIEW_DESC* uavDesc) {
		D3D12_CPU_DESCRIPTOR_HANDLE desc = texture->GetUnorderedAccessViewCPU(miplevel);
		if (desc.ptr != 0) return desc;
		texture->CreateUnorderedAccessView(gDescriptorAllocator.AllocateDescriptor(1), uavDesc, miplevel);
		return texture->GetUnorderedAccessViewCPU();
	}

	D3D12_CPU_DESCRIPTOR_HANDLE ResourceNode::CreateRtvDescriptor(Descriptor desc, D3D12_RENDER_TARGET_VIEW_DESC* rtvDesc) {
		if (texture->GetRenderTargetViewCPU().ptr != 0) {
			return texture->GetRenderTargetViewCPU();
		}
		texture->CreateRenderTargetView(desc, rtvDesc);
		return texture->GetRenderTargetViewCPU();
	}
	D3D12_CPU_DESCRIPTOR_HANDLE ResourceNode::GetRtvDescriptor() {
		return texture->GetRenderTargetViewCPU();
	}

	//create a dsv for the resource node
	D3D12_CPU_DESCRIPTOR_HANDLE ResourceNode::CreateDsvDescriptor(Descriptor desc, D3D12_DEPTH_STENCIL_VIEW_DESC* dsvDesc) {
		if (texture->GetDepthStencilViewCPU().ptr != 0) {
			return texture->GetDepthStencilViewCPU();
		}
		texture->CreateDepthStencilView(desc,dsvDesc);
		return texture->GetDepthStencilViewCPU();
	}
	D3D12_CPU_DESCRIPTOR_HANDLE ResourceNode::GetDsvDescriptor() {
		return texture->GetDepthStencilViewCPU();
	}
}