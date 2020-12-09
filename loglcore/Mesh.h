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

	D3D12_VERTEX_BUFFER_VIEW* GetVBV() { 
		if(isValid) 
			return &vbv;
		return nullptr;
	}
	D3D12_INDEX_BUFFER_VIEW*  GetIBV() { 
		if(isValid)
			return &ibv;
		return nullptr;
	}

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

	bool isValid;
};

class StaticMesh {
public:
	StaticMesh(size_t vertexNum, MeshVertex* vertexs,UploadBatch* batch = nullptr);
	StaticMesh(size_t vertexStride, size_t vertexNum, void* vertexs,UploadBatch* batch = nullptr);
	StaticMesh(size_t indexNum, uint16_t* indices, size_t vertexNum, MeshVertex* vertexs, UploadBatch* batch = nullptr);
	StaticMesh(size_t indexNum, uint16_t* indices, size_t vertexStride, size_t vertexNum, void* vertexs, UploadBatch* batch = nullptr);

	D3D12_VERTEX_BUFFER_VIEW* GetVBV() {
		if (isValid)
			return &vbv;
		return nullptr;
	}
	D3D12_INDEX_BUFFER_VIEW*  GetIBV() {
		if (isValid)
			return &ibv;
		return nullptr;
	}

	size_t GetVertexNum() { return vertexBufferSize; }
	size_t GetIndexNum() { return indexSize; }

	bool IsIndexBufferUsed() { return useIndex; }
	bool IsValid() { return isValid; }

	~StaticMesh() {
		vertexBuffer = nullptr;
		indexBuffer = nullptr;
	}

private:
	D3D12_VERTEX_BUFFER_VIEW vbv;
	ComPtr<ID3D12Resource> vertexBuffer;
	size_t vertexBufferSize;

	bool useIndex;
	D3D12_INDEX_BUFFER_VIEW ibv;
	ComPtr<ID3D12Resource> indexBuffer;
	size_t indexSize;

	bool isValid;
};