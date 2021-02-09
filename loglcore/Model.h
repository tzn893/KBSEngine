#pragma once
#include "SubMesh.h"
#include "SkinnedSubMesh.h"
#include "SubMeshMaterial.h"

#include <functional>

template<typename SM>
class TModel {
public:
	TModel(const char* name) :name(name) {}

	SM* GetSubMesh(size_t index) {
		if (index < subMeshs.size())
			return subMeshs[index].get();
		return nullptr;
	}
	SM* GetSubMesh(const std::string& name) {
		for (auto& item : subMeshs) {
			if (std::string(item->GetName()) == name) {
				return item.get();
			}
		}
		return nullptr;
	}

	void PushBackSubMesh(SM* subMesh) {
		subMeshs.push_back(std::move(std::unique_ptr<SM>(subMesh)));
	}
	void PushBackSubMeshMaterial(SubMeshMaterial material) {
		subMaterials.push_back(material);
	}

	SubMeshMaterial* GetMaterial(SM* subMesh) {
		if (subMesh->GetSubMeshMaterialIndex() >= 0 && subMesh->GetSubMeshMaterialIndex() < subMaterials.size())
			return &subMaterials[subMesh->GetSubMeshMaterialIndex()];
		return nullptr;
	}

	const char* GetName() { return name.c_str(); }

	using SubMeshEnumerator = std::function<void(SM* targetMesh, TModel* model, size_t index)>;

	void ForEachSubMesh(const SubMeshEnumerator& callback) {
		size_t index = 0;
		for (auto& item : subMeshs) {
			callback(item.get(), this, index++);
		}
	}
	size_t GetSubMeshNum() { return subMeshs.size(); }
	size_t GetSubMaterialNum() { return subMaterials.size(); }
private:

	std::vector<std::unique_ptr<SM>> subMeshs;
	std::vector<SubMeshMaterial> subMaterials;
	std::string name;
};

using Model = TModel<SubMesh>;

class SkinnedModel : TModel<SkinnedSubMesh> {
	
};

/*
class Model {
public:
	Model(const char* name) :name(name) {}

	SubMesh* GetSubMesh(size_t index) { 
		if (index < subMeshs.size()) 
			return subMeshs[index].get(); 
		return nullptr;
	}
	SubMesh* GetSubMesh(const std::string& name) {
		for (auto& item : subMeshs) {
			if (std::string(item->GetName()) == name) {
				return item.get();
			}
		}
		return nullptr;
	}
	
	void PushBackSubMesh(SubMesh* subMesh) {
		subMeshs.push_back(std::move(std::unique_ptr<SubMesh>(subMesh)));
	}
	void PushBackSubMeshMaterial(SubMeshMaterial material) {
		subMaterials.push_back(material);
	}

	SubMeshMaterial* GetMaterial(SubMesh* subMesh) {
		if (subMesh->GetSubMeshMaterialIndex() >= 0 && subMesh->GetSubMeshMaterialIndex() < subMaterials.size())
			return &subMaterials[subMesh->GetSubMeshMaterialIndex()];
		return nullptr;
	}
	
	const char* GetName() { return name.c_str(); }

	using SubMeshEnumerator = std::function<void(SubMesh* targetMesh,Model* model,size_t index)>;

	void ForEachSubMesh(const SubMeshEnumerator& callback) {
		size_t index = 0;
		for (auto& item : subMeshs) {
			callback(item.get(), this, index++);
		}
	}
	size_t GetSubMeshNum() { return subMeshs.size(); }
	size_t GetSubMaterialNum() { return subMaterials.size(); }
private:

	std::vector<std::unique_ptr<SubMesh>> subMeshs;
	std::vector<SubMeshMaterial> subMaterials;
	std::string name;
};
*/