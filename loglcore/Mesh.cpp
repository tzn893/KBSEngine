#include "Mesh.h"
#include "graphic.h"

DynamicMesh::DynamicMesh(size_t vertexNum, MeshVertex* vertexs) {
	ID3D12Device* device = gGraphic.GetDevice();
	HRESULT hr = device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(vertexNum * sizeof(MeshVertex)),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertexUploadBuffer));
	if (FAILED(hr)) {
		vertexUploadBuffer = nullptr;
		vertexUploadBufferPtr = nullptr;
	}
	else {
		useIndex = false;
		indexBuffer = nullptr;
		indexSize = 0;

		vertexBufferSize = vertexNum;
		void* writer;
		vertexUploadBuffer->Map(0, nullptr,&writer);
		vertexUploadBufferPtr = reinterpret_cast<MeshVertex*>(writer);

		vbv.BufferLocation = vertexUploadBuffer->GetGPUVirtualAddress();
		vbv.SizeInBytes = vertexBufferSize * sizeof(MeshVertex);
		vbv.StrideInBytes = sizeof(MeshVertex);
	}
}


DynamicMesh::DynamicMesh(size_t indexNum, uint16_t* indices, size_t vertexNum, MeshVertex* vertexs,UploadBatch* batch) {
	ID3D12Device* device = gGraphic.GetDevice();
	HRESULT hr = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(vertexNum * sizeof(MeshVertex)),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertexUploadBuffer));


	if (FAILED(hr)) {
		vertexUploadBuffer = nullptr;
		vertexUploadBufferPtr = nullptr;
	}
	else {
		useIndex = false;
		if (batch == nullptr) {
			UploadBatch mbatch = UploadBatch::Begin();
			indexBuffer = mbatch.UploadBuffer(sizeof(uint16_t) * indexNum, indices);
			mbatch.End();
		}else
			indexBuffer = batch->UploadBuffer(sizeof(uint16_t) * indexNum,indices);
		indexSize = indexNum;
		if (indexBuffer == nullptr) {
			vertexUploadBuffer = nullptr;
			vertexUploadBufferPtr = nullptr;
		}


		vertexBufferSize = vertexNum;
		void* writer;
		vertexUploadBuffer->Map(0, nullptr, &writer);
		vertexUploadBufferPtr = reinterpret_cast<MeshVertex*>(writer);

		vbv.BufferLocation = vertexUploadBuffer->GetGPUVirtualAddress();
		vbv.SizeInBytes = vertexBufferSize * sizeof(MeshVertex);
		vbv.StrideInBytes = sizeof(MeshVertex);

		ibv.BufferLocation = indexBuffer->GetGPUVirtualAddress();
		ibv.SizeInBytes = indexNum * sizeof(uint16_t*);
		ibv.Format = DXGI_FORMAT_R16_UINT;
	}
}

bool DynamicMesh::WriteVertex(MeshVertex* vertex,size_t index, size_t num) {
	if (index > vertexBufferSize) return false;
	memcpy(vertexUploadBufferPtr + index, vertex, num * sizeof(MeshVertex));
}