#pragma once
#include "Mesh.h"
#include "ShaderDataStruct.h"
#include "Texture.h"

enum SUBMESH_MATERIAL_TYPE {
	SUBMESH_MATERIAL_TYPE_DIFFUSE = 0,
	SUBMESH_MATERIAL_TYPE_SPECULAR = 1,
	SUBMESH_MATERIAL_TYPE_BUMP = 2,
	SUBMESH_MATERIAL_TYPE_NUM = 3
};

struct SubMeshMaterial {
	Texture* textures[SUBMESH_MATERIAL_TYPE_NUM];
	Game::Vector3 diffuse;
	float roughness;
	Game::Vector3 specular;
};

class SubMesh : public StaticMesh<MeshVertexNormal> {
public:
	SubMesh(const char* name, size_t materials,
		ID3D12Device* device, size_t vertexNum, MeshVertexNormal* vertexs, UploadBatch* batch = nullptr)
		: StaticMesh<MeshVertexNormal>(device, vertexNum, vertexs, batch), name(name), 
		subMeshMaterialIndex(materials) {}
	
	SubMesh(const char* name, size_t materials, ID3D12Device* device, size_t indexNum, uint16_t* indices
		, size_t vertexNum, MeshVertexNormal* vertexs, UploadBatch* batch = nullptr)
		: StaticMesh<MeshVertexNormal>(device, indexNum, indices, vertexNum, vertexs, batch),
		name(name), subMeshMaterialIndex(materials) {}

	size_t GetSubMeshMaterialIndex() { return subMeshMaterialIndex; }

	const char* GetName() { return name.c_str(); }
private:
	std::string name;
	size_t subMeshMaterialIndex;
};

class Model {
public:
	Model(const char* name) :name(name) {}

	SubMesh* GetSubMesh(size_t index) { 
		if (index < subMeshs.size()) 
			return subMeshs[index].get(); 
		return nullptr;
	}
	SubMesh* GetSubMesh(const char* name) {
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

private:

	std::vector<std::unique_ptr<SubMesh>> subMeshs;
	std::vector<SubMeshMaterial> subMaterials;
	std::string name;
};