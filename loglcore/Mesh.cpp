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
		isValid = false;
	}
	else {
		useIndex = false;
		indexBuffer = nullptr;
		indexSize = 0;

		vertexBufferSize = vertexNum;
		void* writer;
		vertexUploadBuffer->Map(0, nullptr,&writer);
		memcpy(writer, vertexs, vertexNum * sizeof(MeshVertex));
		vertexUploadBufferPtr = reinterpret_cast<MeshVertex*>(writer);

		vbv.BufferLocation = vertexUploadBuffer->GetGPUVirtualAddress();
		vbv.SizeInBytes = vertexBufferSize * sizeof(MeshVertex);
		vbv.StrideInBytes = sizeof(MeshVertex);
		isValid = true;
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
		isValid = false;
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
			isValid = false;
			return;
		}


		vertexBufferSize = vertexNum;
		void* writer;
		vertexUploadBuffer->Map(0, nullptr, &writer);
		memcpy(writer, vertexs, vertexNum * sizeof(MeshVertex));
		vertexUploadBufferPtr = reinterpret_cast<MeshVertex*>(writer);

		vbv.BufferLocation = vertexUploadBuffer->GetGPUVirtualAddress();
		vbv.SizeInBytes = vertexBufferSize * sizeof(MeshVertex);
		vbv.StrideInBytes = sizeof(MeshVertex);

		ibv.BufferLocation = indexBuffer->GetGPUVirtualAddress();
		ibv.SizeInBytes = indexNum * sizeof(uint16_t*);
		ibv.Format = DXGI_FORMAT_R16_UINT;

		isValid = true;
	}
}

bool DynamicMesh::WriteVertex(MeshVertex* vertex,size_t index, size_t num) {
	if (index > vertexBufferSize) return false;
	memcpy(vertexUploadBufferPtr + index, vertex, num * sizeof(MeshVertex));
}

StaticMesh::StaticMesh(size_t vertexNum, MeshVertex* vertexs, UploadBatch* batch) {
	ID3D12Device* device = gGraphic.GetDevice();
	if (batch != nullptr) {
		vertexBuffer = batch->UploadBuffer(vertexNum * sizeof(MeshVertex), vertexs);
		if (vertexBuffer == nullptr) {
			isValid = false;
			return;
		}
	}
	else {
		UploadBatch mbatch = UploadBatch::Begin();
		vertexBuffer = mbatch.UploadBuffer(vertexNum * sizeof(MeshVertex), vertexs);
		if (vertexBuffer == nullptr) {
			isValid = false;
			return;
		}
		else {
			mbatch.End();
		}
	}

	useIndex = false;
	isValid = true;

	vbv.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
	vbv.SizeInBytes = vertexNum * sizeof(MeshVertex);
	vbv.StrideInBytes = sizeof(MeshVertex);

	vertexBufferSize = vertexNum;
}

StaticMesh::StaticMesh(size_t vertexStride, size_t vertexNum, void* vertexs, UploadBatch* batch) {
	ID3D12Device* device = gGraphic.GetDevice();
	if (batch != nullptr) {
		vertexBuffer = batch->UploadBuffer(vertexNum * vertexStride, vertexs);
		if (vertexBuffer == nullptr) {
			isValid = false;
			return;
		}
	}
	else {
		UploadBatch mbatch = UploadBatch::Begin();
		vertexBuffer = mbatch.UploadBuffer(vertexNum * vertexStride, vertexs);
		if (vertexBuffer == nullptr) {
			isValid = false;
			return;
		}
		else {
			mbatch.End();
		}
	}

	useIndex = false;
	isValid = true;

	vbv.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
	vbv.SizeInBytes = vertexNum * vertexStride;
	vbv.StrideInBytes = vertexStride;

	vertexBufferSize = vertexNum;
}

StaticMesh::StaticMesh(size_t indexNum, uint16_t* indices, size_t vertexNum, MeshVertex* vertexs, UploadBatch* batch) {
	ID3D12Device* device = gGraphic.GetDevice();
	if (batch != nullptr) {
		vertexBuffer = batch->UploadBuffer(vertexNum * sizeof(MeshVertex), vertexs);
		if (vertexBuffer == nullptr) {
			isValid = false;
			return;
		}
		indexBuffer = batch->UploadBuffer(indexNum * sizeof(uint16_t), indices);
		if (indexBuffer == nullptr) {
			isValid = false;
			return;
		}
	}
	else {
		UploadBatch mbatch = UploadBatch::Begin();
		vertexBuffer = mbatch.UploadBuffer(vertexNum * sizeof(MeshVertex), vertexs);
		indexBuffer = mbatch.UploadBuffer(indexNum * sizeof(uint16_t), indices);
		if (vertexBuffer == nullptr || indexBuffer == nullptr) {
			isValid = false;
			return;
		}
		else {
			mbatch.End();
		}
	}

	useIndex = false;
	isValid = true;

	vbv.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
	vbv.SizeInBytes = vertexNum * sizeof(MeshVertex);
	vbv.StrideInBytes = sizeof(MeshVertex);

	ibv.BufferLocation = indexBuffer->GetGPUVirtualAddress();
	ibv.SizeInBytes = indexNum * sizeof(uint16_t);
	ibv.Format = DXGI_FORMAT_R16_UINT;

	vertexBufferSize = vertexNum;
	indexSize = indexNum;
}

StaticMesh::StaticMesh(size_t indexNum, uint16_t* indices, size_t vertexStride, size_t vertexNum, void* vertexs, UploadBatch* batch) {
	ID3D12Device* device = gGraphic.GetDevice();
	if (batch != nullptr) {
		vertexBuffer = batch->UploadBuffer(vertexNum * vertexStride, vertexs);
		if (vertexBuffer == nullptr) {
			isValid = false;
			return;
		}
		indexBuffer = batch->UploadBuffer(indexNum * sizeof(uint16_t), indices);
		if (indexBuffer == nullptr) {
			isValid = false;
			return;
		}
	}
	else {
		UploadBatch mbatch = UploadBatch::Begin();
		vertexBuffer = mbatch.UploadBuffer(vertexNum * vertexStride, vertexs);
		indexBuffer = mbatch.UploadBuffer(indexNum * sizeof(uint16_t), indices);
		if (vertexBuffer == nullptr || indexBuffer == nullptr) {
			isValid = false;
			return;
		}
		else {
			mbatch.End();
		}
	}

	useIndex = false;
	isValid = true;

	vbv.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
	vbv.SizeInBytes = vertexNum * vertexStride;
	vbv.StrideInBytes = vertexStride;

	ibv.BufferLocation = indexBuffer->GetGPUVirtualAddress();
	ibv.SizeInBytes = indexNum * sizeof(uint16_t);
	ibv.Format = DXGI_FORMAT_R16_UINT;

	vertexBufferSize = vertexNum;
	indexSize = indexNum;
}