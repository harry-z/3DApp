#include "PixelFormatUtil.h"

dword GetPixelFormatStride(EPixelFormat format) {
	switch (format) {
	case EPixelFormat::EPixelFormat_Unknown:
	case EPixelFormat::EPixelFormat_BC1:
	case EPixelFormat::EPixelFormat_BC2:
	case EPixelFormat::EPixelFormat_BC3:
	case EPixelFormat::EPixelFormat_BC4:
	case EPixelFormat::EPixelFormat_BC5:
	case EPixelFormat::EPixelFormat_BC6H:
	case EPixelFormat::EPixelFormat_BC7:
	case EPixelFormat::EPixelFormat_Depth16:
	case EPixelFormat::EPixelFormat_Depth24:
	case EPixelFormat::EPixelFormat_Depth32:
	case EPixelFormat::EPixelFormat_R9G9B9E5_SharedExp:
	default:
		return 0;
	case EPixelFormat::EPixelFormat_L8:
	case EPixelFormat::EPixelFormat_A8:
		return 1;
	case EPixelFormat::EPixelFormat_A8L8:
	case EPixelFormat::EPixelFormat_F16_R:
		return 2;
	case EPixelFormat::EPixelFormat_A2R10G10B10:
	case EPixelFormat::EPixelFormat_X8R8G8B8:
	case EPixelFormat::EPixelFormat_A8R8G8B8:
#if defined(RENDERAPI_DX9)
	case EPixelFormat::EPixelFormat_R8G8B8:
#endif
	case EPixelFormat::EPixelFormat_F16_RG:
	case EPixelFormat::EPixelFormat_F32_R:
		return 4;
	case EPixelFormat::EPixelFormat_F16_RGBA:
	case EPixelFormat::EPixelFormat_F32_RG:
		return 8;
	case EPixelFormat::EPixelFormat_F32_RGBA:
		return 16;
	}
}

void GetSurfaceInfo(word nWidth, word nHeight, byte nDepth, EPixelFormat eFormat,
	INOUT dword* pNumBytes, INOUT dword* pRowBytes, INOUT dword* pNumRows) {

	bool bCompressed = true;
	dword nbcNumBytesPerBlock = 0;
	switch (eFormat)
	{
	case EPixelFormat::EPixelFormat_BC1:
	case EPixelFormat::EPixelFormat_BC4:
		nbcNumBytesPerBlock = 8;
		break;
	case EPixelFormat::EPixelFormat_BC2:
	case EPixelFormat::EPixelFormat_BC3:
	case EPixelFormat::EPixelFormat_BC5:
	case EPixelFormat::EPixelFormat_BC6H:
	case EPixelFormat::EPixelFormat_BC7:
		nbcNumBytesPerBlock = 16;
		break;
	default:
		bCompressed = false;
		break;
	}

	if (bCompressed)
	{
		dword nqWidth = nWidth / 4;
		dword nqHeight = nHeight / 4;
		dword nBlockWide = 1 > nqWidth ? 1 : nqWidth;
		dword nBlockHigh = 1 > nqHeight ? 1 : nqHeight;
		*pRowBytes = nBlockWide*nbcNumBytesPerBlock;
		*pNumRows = nBlockHigh;
	}
	else
	{
		*pRowBytes = GetPixelFormatStride(eFormat)*nWidth;
		*pNumRows = nHeight;
	}
	*pNumBytes = (*pRowBytes)*(*pNumRows)*nDepth;
}

/*bool HasAlphaComponent(EPixelFormat eFormat) {
	switch (eFormat) {
	case EPixelFormat_Unknown:
	case EPixelFormat_L8:
	case EPixelFormat_Depth16:
	case EPixelFormat_Depth24:
	case EPixelFormat_Depth32:
	case EPixelFormat_R9G9B9E5_SharedExp:
	case EPixelFormat_F16_R:
	case EPixelFormat_F16_RG:
	case EPixelFormat_F32_R:
	case EPixelFormat_F32_RG:
	case EPixelFormat_X8R8G8B8:
	case EPixelFormat_BC1:
	case EPixelFormat_BC4:
	case EPixelFormat_BC5:
	case EPixelFormat_BC6H:
	case EPixelFormat_BC7:
	default:
		return false;
	case EPixelFormat_BC2:
	case EPixelFormat_BC3:
	case EPixelFormat_A8:
	case EPixelFormat_A8L8:
	case EPixelFormat_A2R10G10B10:
	case EPixelFormat_A8R8G8B8:
	case EPixelFormat_F16_RGBA:
	case EPixelFormat_F32_RGBA:
		return true;
	}
}*/