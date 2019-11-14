#include "TextureReader.h"

#include "dds/DDS.h"
#define DDS_BYTES_TO_CHECK 4

#include "png/png.h"
#include "png/pngstruct.h"
#include "png/pnginfo.h"
#define PNG_BYTES_TO_CHECK 4

// #include "ijl.h"
// #pragma comment(lib, "./deps/lib/ijl15.lib")

void TEXTURE_FILE_DESC::Release() {
	if (eFileType == ETextureFileType::ETextureFile_DDS)
		MEMFREE(pData - sizeof(dword) - sizeof(DDS_HEADER));
	else
		MEMFREE(pData);
}



bool CheckIsDDS(const byte *pData, dword nLen) {
	if (nLen < DDS_BYTES_TO_CHECK)
		return false;
	dword dwMagic = *((const dword *)pData);
	return dwMagic == DDS_MAGIC;
}

#define ISBITMASK( r,g,b,a ) ( ddspf.dwRBitMask == r && ddspf.dwGBitMask == g && ddspf.dwBBitMask == b && ddspf.dwABitMask == a )
EPixelFormat GetPixelFormatFromDDS(const DDS_PIXELFORMAT& ddspf)
{
	if (ddspf.dwFlags & DDS_RGB)
	{
		switch (ddspf.dwRGBBitCount)
		{
		case 32:
			if (ISBITMASK(0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000))
				return EPixelFormat::EPixelFormat_A8R8G8B8;
			else if (ISBITMASK(0x00ff0000, 0x0000ff00, 0x000000ff, 0x00000000))
				return EPixelFormat::EPixelFormat_X8R8G8B8;
		}
	}
	else if (ddspf.dwFlags & DDS_LUMINANCE)
	{
		if (8 == ddspf.dwRGBBitCount)
		{
			if (ISBITMASK(0x000000ff, 0x00000000, 0x00000000, 0x00000000))
				return  EPixelFormat::EPixelFormat_L8;
		}
		else if (16 == ddspf.dwRGBBitCount)
		{
			if (ISBITMASK(0x000000ff, 0x00000000, 0x00000000, 0x0000ff00))
				return EPixelFormat::EPixelFormat_A8L8;
		}
	}
	else if (ddspf.dwFlags & DDS_ALPHA)
	{
		if (8 == ddspf.dwRGBBitCount)
		{
			return EPixelFormat::EPixelFormat_A8;
		}
	}
	else if (ddspf.dwFlags & DDS_FOURCC)
	{
		if (MAKE4CC('D', 'X', 'T', '1') == ddspf.dwFourCC)
			return EPixelFormat::EPixelFormat_BC1;
		if (MAKE4CC('D', 'X', 'T', '2') == ddspf.dwFourCC || 
			MAKE4CC('D', 'X', 'T', '3') == ddspf.dwFourCC)
			return EPixelFormat::EPixelFormat_BC2;
		if (MAKE4CC('D', 'X', 'T', '4') == ddspf.dwFourCC || 
			MAKE4CC('D', 'X', 'T', '5') == ddspf.dwFourCC)
			return EPixelFormat::EPixelFormat_BC3;
	}

	return EPixelFormat::EPixelFormat_Unknown;
}

void SwizzleDataDDS(EPixelFormat eFormat, INOUT byte *pData, dword nDataSizeInByte)
{
#ifdef RENDERAPI_DX11
	switch (eFormat)
	{
	// Swizzle RGB to BGR to be DXGI(1.0) supported
	case EPixelFormat_A8R8G8B8:
	case EPixelFormat_X8R8G8B8:
	{
		for (dword i = 0; i < nDataSizeInByte; i += 4)
		{
			byte a = pData[i];
			pData[i] = pData[i + 2];
			pData[i + 2] = a;
		}
	}
	break;
	}
#endif
}

bool Fill_DDS_FILE_DESC(byte *pBuffer, dword nLen, EAutoGenmip bAutoGenMipmap, INOUT TEXTURE_FILE_DESC *pDesc) {
	if (!CheckIsDDS(pBuffer, nLen))
		return false;
	
	DDS_HEADER *pDDSHeader = (DDS_HEADER*)(pBuffer + sizeof(dword));
	if ((pDDSHeader->dwSize != sizeof(DDS_HEADER)) ||
		(pDDSHeader->ddspf.dwSize != sizeof(DDS_PIXELFORMAT)))
		return false;

	if ((pDDSHeader->ddspf.dwFlags == DDS_FOURCC) &&
		(pDDSHeader->ddspf.dwFourCC == MAKE4CC('D', 'X', '1', '0')))
		return false;

	if (pDDSHeader->dwHeaderFlags & DDS_HEADER_FLAGS_VOLUME)
	{
		pDesc->eTexType = ETextureType_3D;
		pDesc->nDepth = pDDSHeader->dwDepth;
	}
	else if (pDDSHeader->dwCubemapFlags != 0)
	{
		pDesc->eTexType = ETextureType_Cube;
		if (pDDSHeader->dwCubemapFlags == DDS_CUBEMAP_ALLFACES)
			pDesc->nFace = ECubeFace_All;
		else
		{
			if (pDDSHeader->dwCubemapFlags & DDS_CUBEMAP_NEGATIVEX)
				pDesc->nFace |= ECubeFace_NegX;
			if (pDDSHeader->dwCubemapFlags & DDS_CUBEMAP_NEGATIVEY)
				pDesc->nFace |= ECubeFace_NegY;
			if (pDDSHeader->dwCubemapFlags & DDS_CUBEMAP_NEGATIVEZ)
				pDesc->nFace |= ECubeFace_NegZ;
			if (pDDSHeader->dwCubemapFlags & DDS_CUBEMAP_POSITIVEX)
				pDesc->nFace |= ECubeFace_PosX;
			if (pDDSHeader->dwCubemapFlags & DDS_CUBEMAP_POSITIVEY)
				pDesc->nFace |= ECubeFace_PosY;
			if (pDDSHeader->dwCubemapFlags & DDS_CUBEMAP_POSITIVEZ)
				pDesc->nFace |= ECubeFace_PosZ;
		}
	}
	else if (pDDSHeader->dwHeight == 1)
		pDesc->eTexType = ETextureType_1D;
	else
		pDesc->eTexType = ETextureType_2D;

	pDesc->nWidth = pDDSHeader->dwWidth;
	pDesc->nHeight = pDDSHeader->dwHeight;


	// Mipmap of dds file is a special case
	if (pDDSHeader->dwHeaderFlags & DDS_HEADER_FLAGS_MIPMAP)
		pDesc->nMipLevel = pDDSHeader->dwMipMapCount;
	else
		pDesc->nMipLevel = (bAutoGenMipmap == EAutoGenmip::EAutoGenmip_AUTO/* && g_ConsoleVars.r_bAutomipIfNeeded)*/ || bAutoGenMipmap == EAutoGenmip::EAutoGenmip_ENABLE) ? 0 : 1;

	pDesc->eFormat = GetPixelFormatFromDDS(pDDSHeader->ddspf);
	pDesc->pData = pBuffer + sizeof(dword) + sizeof(DDS_HEADER);
	SwizzleDataDDS(pDesc->eFormat, pDesc->pData, nLen - sizeof(dword) - sizeof(DDS_HEADER));
	return true;
}

bool ReadDDSFile(const String &szFilePath, EAutoGenmip bAutoGenMipmap, INOUT TEXTURE_FILE_DESC *pDesc) {
	CFile file;
	if (file.Open(szFilePath.c_str(), "rb"))
	{
		dword nLen = file.Length();
		byte *pBuffer = (byte*)MEMALLOC(nLen);
		file.ReadTypeRaw<byte>(pBuffer, nLen);
		file.Close();
		
		if (Fill_DDS_FILE_DESC(pBuffer, nLen, bAutoGenMipmap, pDesc))
			return true;
		else
			MEMFREE(pBuffer);
	}
	return false;
}

bool ReadDDSStream(const byte *pStreamData, dword nLen, EAutoGenmip bAutoGenMipmap, INOUT TEXTURE_FILE_DESC *pDesc) {
	if (pStreamData != nullptr) {
		byte *pBufferCopied = (byte*)MEMALLOC(nLen);
		memcpy(pBufferCopied, pStreamData, nLen);

		if (Fill_DDS_FILE_DESC(pBufferCopied, nLen, bAutoGenMipmap, pDesc))
			return true;
		else
			MEMFREE(pBufferCopied);
	}
	return false;
}


// void SwizzleDataJPEG(INOUT byte *&pData, dword nDataSizeInByte)
// {
// #if defined(RENDERAPI_DX11)

// 	// D3D11
// 	// low bit -- R G B A -- high bit
// 	byte *pNewData = (byte*)MEMALLOC(nDataSizeInByte / 3 * 4);
// 	for (dword i = 0, j = 0; i < nDataSizeInByte; i += 3, j += 4)
// 	{
// 		pNewData[j] = pData[i + 2];
// 		pNewData[j + 1] = pData[i + 1];
// 		pNewData[j + 2] = pData[i];
// 		pNewData[j + 3] = 255;
// 	}
// 	MEMFREE(pData);
// 	pData = pNewData;


// #elif defined(RENDERAPI_DX9)

// 	// D3D9
// 	// low bit -- B G R A -- high bit
// 	byte *pNewData = (byte*)MEMALLOC(nDataSizeInByte / 3 * 4);
// 	for (dword i = 0, j = 0; i < nDataSizeInByte; i += 3, j += 4)
// 	{
// 		pNewData[j] = pData[i];
// 		pNewData[j + 1] = pData[i + 1];
// 		pNewData[j + 2] = pData[i + 2];
// 		pNewData[j + 3] = 255;
// 	}
// 	MEMFREE(pData);
// 	pData = pNewData;

// #else

// #endif
// }

// bool Fill_JPEG_FILE_DESC(byte *pData, dword nLen, EAutoGenmip bAutoGenMipmap,
// 	JPEG_CORE_PROPERTIES &jpeg_core_props,
// 	INOUT TEXTURE_FILE_DESC *pDesc) {
// 	if (ijlInit(&jpeg_core_props) != IJL_OK)
// 		return false;

// 	jpeg_core_props.JPGBytes = pData;
// 	jpeg_core_props.JPGSizeBytes = nLen;

// 	if (ijlRead(&jpeg_core_props, IJL_JBUFF_READPARAMS) != IJL_OK)
// 		return false;

// 	switch (jpeg_core_props.JPGChannels)
// 	{
// 	case 1:
// 		//jpeg_core_props.JPGColor = IJL_G;
// 		jpeg_core_props.DIBColor = IJL_G;
// 		jpeg_core_props.DIBChannels = 1;
// 		break;
// 	case 3:
// 		//jpeg_core_props.JPGColor = IJL_YCBCR;
// 		jpeg_core_props.DIBColor = IJL_BGR;
// 		jpeg_core_props.DIBChannels = 3;
// 		break;
// 	case 4:
// 	default:
// 		//ijlFree(&jpeg_core_props);
// 		//FREE_MEM(pData);
// 		return false;
// 	}

// 	jpeg_core_props.DIBWidth = jpeg_core_props.JPGWidth;
// 	jpeg_core_props.DIBHeight = jpeg_core_props.JPGHeight;
// 	jpeg_core_props.DIBPadBytes = IJL_DIB_PAD_BYTES(jpeg_core_props.DIBWidth, jpeg_core_props.DIBChannels);
// 	dword nSize = (jpeg_core_props.DIBWidth*jpeg_core_props.DIBChannels + jpeg_core_props.DIBPadBytes)*jpeg_core_props.DIBHeight;
// 	byte *pDIBImageData = (byte*)MEMALLOC(nSize);
// 	jpeg_core_props.DIBBytes = pDIBImageData;
// 	if (ijlRead(&jpeg_core_props, IJL_JBUFF_READWHOLEIMAGE) != IJL_OK)
// 	{
// 		MEMFREE(pDIBImageData);
// 		return false;
// 	}

// 	pDesc->nWidth = jpeg_core_props.DIBWidth;
// 	pDesc->nHeight = jpeg_core_props.DIBHeight;
// 	pDesc->eTexType = (jpeg_core_props.DIBHeight == 1) ? ETextureType_1D : ETextureType_2D;
// 	switch (jpeg_core_props.DIBChannels)
// 	{
// 	case 1:
// 		pDesc->pData = pDIBImageData;
// 		pDesc->eFormat = EPixelFormat_L8;
// 		break;
// 	case 3:
// 		SwizzleDataJPEG(pDIBImageData, nSize);
// 		pDesc->pData = pDIBImageData;
// 		pDesc->eFormat = EPixelFormat_X8R8G8B8;
// 		break;
// 	}

// 	ijlFree(&jpeg_core_props);
// 	pDesc->nMipLevel = ((bAutoGenMipmap == EAutoGenmip_AUTO && g_ConsoleVars.r_bAutomipIfNeeded) || bAutoGenMipmap == EAutoGenmip_ENABLE) ? 0 : 1;
// 	return true;
// }

// bool ReadJPEGFile(const String &szFilePath, EAutoGenmip bAutoGenMipmap, INOUT TEXTURE_FILE_DESC *pDesc) 
// {
// 	CFile file;
// 	if (file.Open(szFilePath.c_str(), "rb"))
// 	{
// 		dword nLen = file.Length();
// 		byte *pData = (byte*)MEMALLOC(nLen);
// 		file.ReadTypeRaw<byte>(pData, nLen);
// 		file.Close();

// 		JPEG_CORE_PROPERTIES jpeg_core_props;
// 		if (Fill_JPEG_FILE_DESC(pData, nLen, bAutoGenMipmap, jpeg_core_props, pDesc)) {
// 			MEMFREE(pData);
// 			return true;
// 		}
// 		else {
// 			MEMFREE(pData);
// 			ijlFree(&jpeg_core_props);
// 		}
// 	}
// 	return false;
// }

// bool ReadJPEGStream(const byte *pStreamData, dword nLen, EAutoGenmip bAutoGenMipmap,
// 	INOUT TEXTURE_FILE_DESC *pDesc) 
// {
// 	assert(pStreamData != nullptr && nLen > 0);
// 	JPEG_CORE_PROPERTIES jpeg_core_props;
// 	if (Fill_JPEG_FILE_DESC(const_cast<byte*>(pStreamData), nLen, bAutoGenMipmap, jpeg_core_props, pDesc))
// 		return true;
// 	else
// 		ijlFree(&jpeg_core_props);
// }



void SwizzleDataPNG(EPixelFormat eFormat, INOUT byte *pData, png_infop info_ptr, png_bytep *row_pointers)
{
#if defined(RENDERAPI_DX11)

	switch (eFormat)
	{
		case EPixelFormat::EPixelFormat_X8R8G8B8:
		{
			for (png_uint_32 i = 0; i < info_ptr->height; ++i)
			{
				byte *temp = row_pointers[i];
				for (png_uint_32 j = 0; j < info_ptr->width; ++j)
				{
					*pData++ = *(temp + 2);
					*pData++ = *(temp + 1);
					*pData++ = *temp;
					*pData++ = 255;
					temp += 3;
				}
			}
		}
		break;
		case EPixelFormat::EPixelFormat_A8R8G8B8:
		{
			for (png_uint_32 i = 0; i < info_ptr->height; ++i)
			{
				byte *temp = row_pointers[i];
				for (png_uint_32 j = 0; j < info_ptr->width; ++j)
				{
					*pData++ = *(temp + 2);
					*pData++ = *(temp + 1);
					*pData++ = *temp;
					*pData++ = *(temp + 3);
					temp += 4;
				}
			}
		}
		break;
	}

#elif defined(RENDERAPI_DX9)

	switch (eFormat)
	{
		case EPixelFormat::EPixelFormat_X8R8G8B8:
		{
			for (png_uint_32 i = 0; i < info_ptr->height; ++i)
			{
				byte *temp = row_pointers[i];
				for (png_uint_32 j = 0; j < info_ptr->width; ++j)
				{
					*pData++ = *temp++;
					*pData++ = *temp++;
					*pData++ = *temp++;
					*pData++ = 255;
				}
			}
		}
		break;
		case EPixelFormat::EPixelFormat_A8R8G8B8:
		{
			for (png_uint_32 i = 0; i < info_ptr->height; ++i)
			{
				memcpy(pData, row_pointers[i], info_ptr->rowbytes);
				pData += info_ptr->rowbytes;
			}
		}
		break;
	}

#endif
}

bool CheckIsPNG(const byte *pData, dword nLen)
{
	if (nLen < PNG_BYTES_TO_CHECK)
		return false;
	return png_sig_cmp(pData, 0, PNG_BYTES_TO_CHECK) == 0;
}

// Call back handlers
//1) Error Handler
void LibPNG_ErrorHandler(png_structp pPNGStruct, png_const_charp pMsg)
{
	Global::m_pLog->Log(ELogType::eLogType_Error, ELogFlag::eLogFlag_Critical, pMsg);
}
// 2) Warning Handler
void LibPNG_WarningHandler(png_structp pPNGStruct, png_const_charp pMsg)
{
	(void)pPNGStruct;
}
// 3) Malloc Func
png_voidp LibPNG_MallocFunc(png_structp pPNGStruct, png_size_t nSize)
{
	return MEMALLOC(nSize);
}
// 4) Free Func
void LibPNG_FreeFunc(png_structp pPNGStruct, png_voidp pBuf)
{
	MEMFREE(pBuf);
}
// 5) RW Func
void LibPNG_IOFunc(png_structp pPNGStruct, png_bytep pBufPointer, png_size_t nReadLength)
{
	byte *p = (byte*)(pPNGStruct->io_ptr);
	memcpy(pBufPointer, p, nReadLength);
	p += nReadLength;
	pPNGStruct->io_ptr = p;
}

bool Fill_PNG_FILE_DESC(byte *pData, dword nLen, EAutoGenmip bAutoGenMipmap,
	INOUT TEXTURE_FILE_DESC *pDesc) 
{
	if (!CheckIsPNG(pData, nLen))
		return false;

	png_structp png_ptr;
	png_infop info_ptr;
	png_ptr = png_create_read_struct_2(PNG_LIBPNG_VER_STRING,
		0, // error_ptr
		LibPNG_ErrorHandler, // error_func
		LibPNG_WarningHandler, // warning_func
		0, // mem_ptr
		LibPNG_MallocFunc, // malloc_func
		LibPNG_FreeFunc); // free_func

	if (png_ptr == 0)
		return false;

	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == 0)
	{
		png_destroy_read_struct(&png_ptr, 0, 0);
		return false;
	}

	png_set_sig_bytes(png_ptr, 8); // 跳过PNG签名

	byte *pBufPtr = pData;
	pBufPtr += 8;
	png_set_read_fn(png_ptr, (png_voidp)pBufPtr, LibPNG_IOFunc);

	png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_PACKSWAP | PNG_TRANSFORM_EXPAND | PNG_TRANSFORM_BGR | PNG_TRANSFORM_SWAP_ENDIAN, 0);
	png_bytep *row_pointers = png_get_rows(png_ptr, info_ptr);

	dword nNumBytes = 0;
	switch (png_ptr->channels)
	{
		case 1:
			pDesc->eFormat = EPixelFormat::EPixelFormat_L8;
			nNumBytes = info_ptr->rowbytes*info_ptr->height;
			break;
		case 3:
			pDesc->eFormat = EPixelFormat::EPixelFormat_X8R8G8B8;
			nNumBytes = info_ptr->width * 4 * info_ptr->height;
			break;
		case 4:
			pDesc->eFormat = EPixelFormat::EPixelFormat_A8R8G8B8;
			nNumBytes = info_ptr->rowbytes*info_ptr->height;
			break;
		default:
		{
			png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
			return false;
		}
	}

	pDesc->nWidth = info_ptr->width;
	pDesc->nHeight = info_ptr->height;
	pDesc->eTexType = (info_ptr->height == 1) ? ETextureType_1D : ETextureType_2D;
	byte *pImageData = (byte *)MEMALLOC(nNumBytes);
	byte *pRow = pImageData;
	switch (pDesc->eFormat)
	{
		case EPixelFormat::EPixelFormat_L8:
		{
			for (png_uint_32 i = 0; i < info_ptr->height; ++i)
			{
				memcpy(pRow, info_ptr->row_pointers[i], info_ptr->rowbytes);
				pRow += info_ptr->rowbytes;
			}
			break;
		}
		case EPixelFormat::EPixelFormat_X8R8G8B8:
		case EPixelFormat::EPixelFormat_A8R8G8B8:
			SwizzleDataPNG(pDesc->eFormat, pRow, info_ptr, row_pointers);
			break;
	}
	pDesc->pData = pImageData;

	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	pDesc->nMipLevel = (bAutoGenMipmap == EAutoGenmip::EAutoGenmip_AUTO/* && g_ConsoleVars.r_bAutomipIfNeeded)*/ || bAutoGenMipmap == EAutoGenmip::EAutoGenmip_ENABLE) ? 0 : 1;
	return true;
}

bool ReadPNGFile(const String &szFilePath, EAutoGenmip bAutoGenMipmap, INOUT TEXTURE_FILE_DESC *pDesc) 
{
	CFile file;
	if (file.Open(szFilePath.c_str(), "rb"))
	{
		dword nLen = file.Length();
		byte *pData = (byte *)MEMALLOC(nLen);
		file.ReadTypeRaw<byte>(pData, nLen);
		file.Close();

		if (Fill_PNG_FILE_DESC(pData, nLen, bAutoGenMipmap, pDesc)) {
			MEMFREE(pData);
			return true;
		}
		else 
			MEMFREE(pData);
	}
	return false;
}

bool ReadPNGStream(const byte *pStreamData, dword nLen, EAutoGenmip bAutoGenMipmap,
	INOUT TEXTURE_FILE_DESC *pDesc) 
{
	assert(pStreamData != nullptr && nLen > 0);
	return Fill_PNG_FILE_DESC(const_cast<byte*>(pStreamData), nLen, bAutoGenMipmap, pDesc);
}

bool ReadTextureFile(const String &szFilePath, EAutoGenmip bAutoGenMipmap, INOUT TEXTURE_FILE_DESC *pDesc) 
{
	String::size_type pos = szFilePath.rfind('.');
	if (pos == String::npos)
		return false;
	String szExt = szFilePath.substr(pos + 1);
	if (szExt.empty())
		return false;
	if (stricmp(szExt.c_str(), "dds") == 0) {
		pDesc->eFileType = ETextureFileType::ETextureFile_DDS;
		return ReadDDSFile(szFilePath, bAutoGenMipmap, pDesc);
	}
	else if (stricmp(szExt.c_str(), "png") == 0) {
		pDesc->eFileType = ETextureFileType::ETextureFile_PNG;
		return ReadPNGFile(szFilePath, bAutoGenMipmap, pDesc);
	}
	// else if (stricmp(szExt.c_str(), "jpg") == 0 || stricmp(szExt.c_str(), "jpeg") == 0) {
	// 	pDesc->eFileType = ETextureFileType::ETextureFile_JPG;
	// 	return ReadJPEGFile(szFilePath, bAutoGenMipmap, pDesc);
	// }
	return false;
}

bool ReadTextureStream(const byte *pStreamData, dword nLen, EAutoGenmip bAutoGenMipmap, 
	INOUT TEXTURE_FILE_DESC *pDesc) 
{
	if (pStreamData != nullptr && nLen > 0) {
		if (CheckIsDDS(pStreamData, nLen)) {
			pDesc->eFileType = ETextureFileType::ETextureFile_DDS;
			return ReadDDSStream(pStreamData, nLen, bAutoGenMipmap, pDesc);
		}
		else if (CheckIsPNG(pStreamData, nLen)) {
			pDesc->eFileType = ETextureFileType::ETextureFile_PNG;
			return ReadPNGStream(pStreamData, nLen, bAutoGenMipmap, pDesc);
		}
		// else {
		// 	pDesc->eFileType = ETextureFileType::ETextureFile_JPG;
		// 	return ReadJPEGStream(pStreamData, nLen, bAutoGenMipmap, pDesc);
		// }
	}
	return false;
}