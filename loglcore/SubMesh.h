#pragma once
#include "Mesh.h"
#include "ShaderDataStruct.h"

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