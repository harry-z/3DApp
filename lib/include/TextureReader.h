#pragma once

#include "Prereq.h"
#include "TextureReaderData.h"

enum class EAutoGenmip : byte {
	EAutoGenmip_ENABLE, // Force auto generating mipmaps
	EAutoGenmip_DISABLE, // Force not to generating mipmaps
	EAutoGenmip_AUTO // Decided by texture file format and console var r_bAutomipIfNeeded
};

bool ReadTextureFile(
	const String &szFilePath,
	EAutoGenmip bAutoGenMipmap,
	INOUT TEXTURE_FILE_DESC *pDesc);

bool ReadTextureStream(
	const byte *pStreamData,
	dword nLen,
	EAutoGenmip bAutoGenMipmap,
	INOUT TEXTURE_FILE_DESC *pDesc);