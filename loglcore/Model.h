#pragma once
#include "SubMesh.h"
#include "SkinnedSubMesh.h"
#include "SubMeshMaterial.h"
#include "BoneHeirarchy.h"

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

class SkinnedModel : public TModel<SkinnedSubMesh> {
public:
	SkinnedModel(BoneHeirarchy* boneHeir,const std::vector<BoneAnimationClip*>& animationClips
		,const char* name):TModel<SkinnedSubMesh>(name) {
		this->boneHeir = std::unique_ptr<BoneHeirarchy>(boneHeir);
		for (auto anima : animationClips) {
			if (anima->GetBoneHeirarchy() == boneHeir&&
				animations.count(anima->GetName()) == 0) {
				animations[anima->GetName()] = std::unique_ptr<BoneAnimationClip>(anima);
			}
			else {
				std::string msg = std::string("animation clip ") + anima->GetName() + "'s bone heirarchy"
							"doesn't match the model's bone heirarchy while constructing the model" + name + "\n";
				OUTPUT_DEBUG_STRING(msg.c_str());
			}
		}
	}

	inline BoneHeirarchy* GetBoneHeirarchy() { return boneHeir.get(); }

	BoneAnimationClip* GetBoneAnimationClip(const char* name) {
		if (auto item = animations.find(name);item != animations.end()) {
			return item->second.get();
		}
		return nullptr;
	}
private:
	std::map<std::string, std::unique_ptr<BoneAnimationClip>> animations;
	std::unique_ptr<BoneHeirarchy> boneHeir;
};