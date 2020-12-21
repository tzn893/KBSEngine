#pragma once
#include <utility>

class NetBuffer {
public:
	NetBuffer(size_t sizeByByte = 0) {
		if (sizeByByte == 0) {
			size = 0;
			buffer = nullptr;
		}
		else {
			capacity = sizeByByte;
			size = sizeByByte;
			buffer = new char[sizeByByte];
		}
	}
	~NetBuffer() {
		delete[] buffer;
	}

	NetBuffer(const NetBuffer&) = delete;
	void operator=(const NetBuffer&) = delete;

	NetBuffer(NetBuffer&& nbuffer) {
		capacity = nbuffer.capacity;
		size = nbuffer.size;
		buffer = nbuffer.buffer;
		nbuffer.size = 0;
		nbuffer.buffer = nullptr;
	}
	void operator=(NetBuffer&& nbuffer) {
		std::swap(nbuffer.capacity, capacity);
		std::swap(size, nbuffer.size);
		std::swap(buffer, nbuffer.buffer);
	}

	void Resize(size_t newSize) {
		if (newSize > capacity) {
			Clear();
			capacity = newSize;
			size = newSize;
			buffer = new char[capacity];
		}
		else {
			size = newSize;
		}
	}

	void Clear() {
		if (buffer != nullptr) {
			delete[] buffer;
			size = 0;
			capacity = 0;
			buffer = nullptr;
		}
	}
	
	template<typename T>
	T* Get(size_t offset_by_byte) { 
		if(offset_by_byte + sizeof(T) <= size)
			return reinterpret_cast<T*>(buffer + offset_by_byte);
		return nullptr;
	}
	size_t GetSize() { return size; }
private:
	char* buffer;
	size_t size;
	size_t capacity;
};