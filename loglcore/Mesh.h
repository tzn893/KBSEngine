#pragma once

#include "d3dcommon.h"

#include "Math.h"
#include "CopyBatch.h"

//Mesh's vertex data can be changed dynamicly
template<typename T>
class DynamicMesh {
public:
	DynamicMesh(ID3D12Device* device,size_t vertexNum, T* vertexs) {
		HRESULT hr = device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(vertexNum * sizeof(T)),
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
			vertexUploadBuffer->Map(0, nullptr, &writer);
			memcpy(writer, vertexs, vertexNum * sizeof(T));
			vertexUploadBufferPtr = reinterpret_cast<T*>(writer);

			vbv.BufferLocation = vertexUploadBuffer->GetGPUVirtualAddress();
			vbv.SizeInBytes = vertexBufferSize * sizeof(T);
			vbv.StrideInBytes = sizeof(T);
			isValid = true;
		}
	}
	DynamicMesh(ID3D12Device* device,size_t indexNum, uint16_t* indices, size_t vertexNum, T* vertexs,UploadBatch* batch = nullptr) {
		HRESULT hr = device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(vertexNum * sizeof(T)),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&vertexUploadBuffer));


		if (FAILED(hr)) {
			vertexUploadBuffer = nullptr;
			vertexUploadBufferPtr = nullptr;
			isValid = false;
		}
		else {
			useIndex = true;
			if (batch == nullptr) {
				UploadBatch mbatch = UploadBatch::Begin();
				indexBuffer = mbatch.UploadBuffer(sizeof(uint16_t) * indexNum, indices);
				mbatch.End();
			}
			else
				indexBuffer = batch->UploadBuffer(sizeof(uint16_t) * indexNum, indices);
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
			memcpy(writer, vertexs, vertexNum * sizeof(T));
			vertexUploadBufferPtr = reinterpret_cast<T*>(writer);

			vbv.BufferLocation = vertexUploadBuffer->GetGPUVirtualAddress();
			vbv.SizeInBytes = vertexBufferSize * sizeof(T);
			vbv.StrideInBytes = sizeof(T);

			ibv.BufferLocation = indexBuffer->GetGPUVirtualAddress();
			ibv.SizeInBytes = indexNum * sizeof(uint16_t);
			ibv.Format = DXGI_FORMAT_R16_UINT;

			isValid = true;
		}
	}

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

	bool WriteVertex(T* vertex, size_t index, size_t num) {
		if (index > vertexBufferSize) return false;
		memcpy(vertexUploadBufferPtr + index, vertex, num * sizeof(T));
		return true;
	}

	T*   GetVertex(size_t index) {
		if (index > vertexBufferSize) return nullptr;
		return vertexUploadBufferPtr + index;
	}

	size_t GetVertexNum() { return vertexBufferSize; }
	size_t GetIndexNum() { return indexSize; }

	ID3D12Resource* GetVertexResource() { return vertexUploadBuffer.Get(); }
	ID3D12Resource* GetIndexResource() { return indexBuffer.Get(); }

	~DynamicMesh() {
		vertexUploadBuffer->Unmap(0, nullptr);
		vertexUploadBuffer = nullptr;
		indexBuffer = nullptr;
	}
private:
	D3D12_VERTEX_BUFFER_VIEW vbv;
	ComPtr<ID3D12Resource> vertexUploadBuffer;
	//the upload buffer memory writer
	T* vertexUploadBufferPtr;
	size_t vertexBufferSize;

	bool useIndex;
	D3D12_INDEX_BUFFER_VIEW ibv;
	ComPtr<ID3D12Resource> indexBuffer;
	size_t indexSize;

	bool isValid;
};

template<typename T>
class StaticMesh {
public:
	StaticMesh(ID3D12Device* device,size_t vertexNum, T* vertexs, UploadBatch* batch = nullptr) {
		if (batch != nullptr) {
			vertexBuffer = batch->UploadBuffer(vertexNum * sizeof(T), vertexs);
			if (vertexBuffer == nullptr) {
				isValid = false;
				return;
			}
		}
		else {
			UploadBatch mbatch = UploadBatch::Begin();
			vertexBuffer = mbatch.UploadBuffer(vertexNum * sizeof(T), vertexs);
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
		vbv.SizeInBytes = vertexNum * sizeof(T);
		vbv.StrideInBytes = sizeof(T);

		vertexBufferSize = vertexNum;
	}


	StaticMesh(ID3D12Device* device,size_t indexNum, uint16_t* indices, size_t vertexNum, T* vertexs, UploadBatch* batch = nullptr) {
		if (batch != nullptr) {
			vertexBuffer = batch->UploadBuffer(vertexNum * sizeof(T), vertexs);
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
			vertexBuffer = mbatch.UploadBuffer(vertexNum * sizeof(T), vertexs);
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
		vbv.SizeInBytes = vertexNum * sizeof(T);
		vbv.StrideInBytes = sizeof(T);

		ibv.BufferLocation = indexBuffer->GetGPUVirtualAddress();
		ibv.SizeInBytes = indexNum * sizeof(uint16_t);
		ibv.Format = DXGI_FORMAT_R16_UINT;

		vertexBufferSize = vertexNum;
		indexSize = indexNum;
	}

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

	ID3D12Resource* GetVertexResource() { return vertexBuffer.Get(); }
	ID3D12Resource* GetIndexResource() { return indexBuffer.Get(); }

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