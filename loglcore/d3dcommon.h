#pragma once
#if (_MSC_VER >= 1915)
#define no_init_all deprecated
#endif

#include <d3d12.h>
#include <dxgi1_4.h>
#include <d3dcompiler.h>

#include <wrl.h>
#include "d3d12x.h"

template<typename T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

#ifdef _DEBUG
#define OUTPUT_DEBUG_STRINGW(str) OutputDebugStringW(str);
#define OUTPUT_DEBUG_STRINGA(str) OutputDebugStringA(str);
#define OUTPUT_DEBUG_STRING(str) OUTPUT_DEBUG_STRINGA(str)
#else
#define OUTPUT_DEBUG_STRINGW(str) (str);
#define OUTPUT_DEBUG_STRINGA(str) (str);
#define OUTPUT_DEBUG_STRING(str) (str);
#endif