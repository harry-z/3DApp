#pragma once

#include "RendererTypes.h"

//struct DDS_FILE_DESC {
//	// Texture type from DDS file
//	ETextureType eTexType;
//	// Texture format from DDS file
//	EPixelFormat eFormat;
//	// Texture width from DDS file
//	word nWidth;
//	// Texture height from DDS file
//	word nHeight;
//	// If this is a volume texture, this variable stores the depth of
//	// the texture. Otherwise, it stores the face flag of the cube texture
//	union {
//		byte nDepth;
//		byte nFace; // Combination of face flags
//	};
//	// Mip level from DDS file, value 1 means no mips
//	byte nMipLevel;
//	// Stores the header pointer of the DDS file
//	byte *pHeader;
//	// The start address of the texture data in the DDS file
//	byte *pData;
//
//	void Release() {
//		if (pHeader)
//			MEMFREE(pHeader);
//	}
//};
//
//struct PNG_JPEG_FILE_DESC {
//	// Texture type from PNG or JPEG file
//	// Only 1D and 2D types are valid for jpeg and png files
//	ETextureType eTexType;
//	// Texture format from PNG or JPEG file
//	EPixelFormat eFormat;
//	// Texture width from PNG or JPEG file
//	word nWidth;
//	// Texture height from PNG or JPEG file
//	word nHeight;
//	// If loading png or jpeg with bAutoGenMipmap true value, then
//	// this param is 0, otherwise it is 1
//	byte nMipLevel;
//	// The start address of the texture data in the PNG or JPEG file
//	byte *pData;
//
//	void Release() {
//		if (pData)
//			MEMFREE(pData);
//	}
//};

enum class ETextureFileType {
	ETextureFile_Unknown,
	ETextureFile_PNG,
	// ETextureFile_JPG,
	ETextureFile_DDS
};

struct TEXTURE_FILE_DESC {
	ETextureFileType eFileType;
	ETextureType eTexType;
	EPixelFormat eFormat;
	word nWidth;
	word nHeight;
	union {
		byte nDepth;
		byte nFace; // CubeMap面标记
	};
	byte nMipLevel;
	byte *pData;

	TEXTURE_FILE_DESC()
		: eFileType(ETextureFileType::ETextureFile_Unknown),
		eTexType(ETextureType_Unknown),
		eFormat(EPixelFormat::EPixelFormat_Unknown),
		nWidth(0),
		nHeight(0),
		nDepth(0),
		nMipLevel(0),
		pData(nullptr) {}

	void Release();
};