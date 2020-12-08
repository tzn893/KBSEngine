#pragma once

#include "d3dcommon.h"

#include "Math.h"
#include "CopyBatch.h"

struct MeshVertex {
	Game::Vector3 Position;
	Game::Vector3 Normal;
	Game::Vector2 TexCoord;
};

//Mesh's vertex data can be changed dynamicly
class DynamicMesh {
public:
	DynamicMesh(size_t vertexNum, MeshVertex* vertexs);
	DynamicMesh(size_t indexNum, uint16_t* indices,size_t vertexNum, MeshVertex* vertexs,UploadBatch* batch = nullptr);

	D3D12_VERTEX_BUFFER_VIEW GetVBV() { return vbv; }
	D3D12_INDEX_BUFFER_VIEW  GetIBV() { return ibv; }

	bool WriteVertex(MeshVertex* vertex,size_t index,size_t num = 1);

	size_t GetVertexNum() { return vertexBufferSize; }
	size_t GetIndexNum() { return indexSize; }

	~DynamicMesh() {
		vertexUploadBuffer->Unmap(0, nullptr);
		vertexUploadBuffer = nullptr;
		indexBuffer = nullptr;
	}
private:
	D3D12_VERTEX_BUFFER_VIEW vbv;
	ComPtr<ID3D12Resource> vertexUploadBuffer;
	//the upload buffer memory writer
	MeshVertex* vertexUploadBufferPtr;
	size_t vertexBufferSize;

	bool useIndex;
	D3D12_INDEX_BUFFER_VIEW ibv;
	ComPtr<ID3D12Resource> indexBuffer;
	size_t indexSize;
};