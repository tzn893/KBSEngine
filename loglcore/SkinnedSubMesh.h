#pragma once
#include "ShaderDataStruct.h"
#include "Mesh.h"

class SkinnedSubMesh : public StaticMesh<SkinnedNormalVertex> {
public:
	SkinnedSubMesh(const char* name, size_t materials,
		ID3D12Device* device, size_t vertexNum, SkinnedNormalVertex* vertexs, UploadBatch* batch = nullptr)
		: StaticMesh<SkinnedNormalVertex>(device, vertexNum, vertexs, batch), name(name),
		subMeshMaterialIndex(materials) {}

	SkinnedSubMesh(const char* name, size_t materials, ID3D12Device* device, size_t indexNum, uint16_t* indices
		, size_t vertexNum, SkinnedNormalVertex* vertexs, UploadBatch* batch = nullptr)
		: StaticMesh<SkinnedNormalVertex>(device, indexNum, indices, vertexNum, vertexs, batch),
		name(name), subMeshMaterialIndex(materials) {}

	size_t GetSubMeshMaterialIndex() { return subMeshMaterialIndex; }

	const char* GetName() { return name.c_str(); }
private:
	std::string name;
	size_t subMeshMaterialIndex;
};
