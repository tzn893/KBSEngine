#pragma once
#include "d3dcommon.h"

template<size_t size>
struct Num32 {
	UINT value[size];
};

template<typename T>
UINT Pack32bitNum(T num) {
	static_assert(sizeof(T) == sizeof(UINT),"Pack32bitNum:the size of the value to be pack should be 4");
	return *reinterpret_cast<UINT*>(&num);
}
