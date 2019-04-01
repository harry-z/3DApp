#pragma once
#include "PreHeader.h"

enum class EPixelFormat : byte {
	EPixelFormat_Unknown,
	EPixelFormat_L8,
	EPixelFormat_A8L8,
	EPixelFormat_A8,
	EPixelFormat_A8R8G8B8,
	EPixelFormat_X8R8G8B8,
	EPixelFormat_R8G8B8,
	EPixelFormat_A2R10G10B10,
	EPixelFormat_F16_R,
	EPixelFormat_F16_RG,
	EPixelFormat_F16_RGBA,
	EPixelFormat_F32_R,
	EPixelFormat_F32_RG,
	EPixelFormat_F32_RGBA,
	EPixelFormat_R9G9B9E5_SharedExp,
	EPixelFormat_Depth16,
	EPixelFormat_Depth24,
	EPixelFormat_Depth32,
	// Three color channels (5:6:5), we only accept BC1 textures without alpha
	EPixelFormat_BC1,
	// Three color channels (5:6:5) with 4 bits of alpha
	EPixelFormat_BC2,
	// Three color channels (5:6:5) with 8 bits of alpha
	EPixelFormat_BC3,
	// One color channel 8 bits
	EPixelFormat_BC4,
	// Two color channels (8:8)
	EPixelFormat_BC5,
	// Three color channels (16:16:16) in half floating point
	EPixelFormat_BC6H,
	// Three color channels (4-7 bits) with 0-8 bits of alpha, we only accept BC7 textures without alpha
	EPixelFormat_BC7
};

void GetSurfaceInfo(word nWidth, word nHeight, byte nDepth, EPixelFormat eFormat,
	INOUT dword* pNumBytes, INOUT dword* pRowBytes, INOUT dword* pNumRows);

