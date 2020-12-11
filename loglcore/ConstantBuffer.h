#pragma once
#include "d3dcommon.h"

template<typename T>
class ConstantBuffer {
public:
	ConstantBuffer(ID3D12Device* mDevice,size_t num = 1, T* data = nullptr) {
		if (num == 0) {
			isValid = true;
			return;
		}
		HRESULT hr = mDevice->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(num * elementSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&buffer)
		);
		if (FAILED(hr)) {
			isValid = false;
			return;
		}

		this->num = num;

		void* ptr;
		buffer->Map(0, nullptr, &ptr);
		bufferPtr = reinterpret_cast<uint8_t*>(ptr);
		isValid = true;

		if (data != nullptr) {
			for (size_t i = 0; i != num; i++) {
				memcpy(bufferPtr + i * elementSize, data + i, sizeof(T));
			}
		}
	}

	T* GetBufferPtr(size_t index = 0){
		if (!isValid || index >= num)
			return nullptr;
		return reinterpret_cast<T*>(bufferPtr + index * elementSize);
	}
	size_t BufferSize(){
		return elementSize * num;
	}

	ID3D12Resource* GetBuffer() {
		if (!isValid)
			return nullptr;
		return buffer.Get();
	}

	D3D12_GPU_VIRTUAL_ADDRESS GetADDR(size_t index = 0) {
		if (index >= num || !isValid)
			return (D3D12_GPU_VIRTUAL_ADDRESS)0;
		return buffer->GetGPUVirtualAddress() + index * elementSize;
	}

	void Resize(ID3D12Device* mDevice,size_t num) {
		size_t oldSize = this->num * elementSize;
		size_t newSize = num * elementSize;

		size_t bufferSize = oldSize > newSize ? newSize : oldSize;
		void* membuffer = malloc(bufferSize);

		memcpy(membuffer, this->bufferPtr, bufferSize);

		buffer->Unmap(0, nullptr);
		buffer = nullptr;

		HRESULT hr = mDevice->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(newSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&buffer)
		);
		if (FAILED(hr)) {
			free(membuffer);
			isValid = false;
			return;
		}

		this->num = num;

		void* ptr;
		buffer->Map(0, nullptr, &ptr);
		bufferPtr = reinterpret_cast<uint8_t*>(ptr);
		isValid = true;

		memcpy(bufferPtr, membuffer, bufferSize);

		free(membuffer);
	}

	~ConstantBuffer() {
		buffer->Unmap(0, nullptr);
		buffer = nullptr;
	}
private:
	uint8_t* bufferPtr;
	static constexpr size_t elementSize = (sizeof(T) + 255) & (~255);
	size_t num;
	ComPtr<ID3D12Resource> buffer;
	bool isValid;
};