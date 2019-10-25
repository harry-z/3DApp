#include "JobSystem.h"
#include "Texture.h"
#include "RenderBackend.h"

void CTexture::Destroy() 
{
	Global::m_pTextureManager->DestroyTexture(this);
}

bool CTexture::Streaming(const String &sFilePath, EAutoGenmip bAutoGenMipmap, INOUT TEXTURE_FILE_DESC *pDesc) 
{
	return ReadTextureFile(sFilePath, bAutoGenMipmap, pDesc);
}


REGISTER_JOB_CTOR_DTOR_IMPL(TEXTURE_LOADING_JOB);

void TextureLoadingJob::OnJobFailed() 
{
	TextureLoadingJobData *pJobData = (TextureLoadingJobData*)m_pData;
	DELETE_TYPE(pJobData, TextureLoadingJobData);
}

void TextureLoadingJob::OnJobFinished() 
{
	OnJobFailed();
}

bool TextureLoadingJob::DoWork_WorkerThread()
{
	TextureLoadingJobData *pJobData = (TextureLoadingJobData*)m_pData;
	return pJobData->pTexture->Streaming(pJobData->szFileName, pJobData->bAutoGenMipmap, &pJobData->texture_desc);
}

bool TextureLoadingJob::DoWork_MainThread() 
{
	TextureLoadingJobData *pJobData = (TextureLoadingJobData*)m_pData;
	pJobData->pTexture->Load(&pJobData->texture_desc, pJobData->bGamma);
	pJobData->pTexture->m_pJob = nullptr;
	return true;
}

CTextureManager::CTextureManager() 
: m_TextureId(1) 
{}

CTexture* CTextureManager::CreateTexture(const String& szName, word width, word height, 
	ETextureType textureType /* = ETextureType_2D */, 
	EPixelFormat pixelFormat /* = EPixelFormat_X8R8G8B8 */, 
	ETextureUsage eUsage /* = ETextureUsage_Dynamic */) 
{
	assert(Global::IsMainThread() && eUsage != ETextureUsage::ETextureUsage_Default);
	assert((textureType == ETextureType_1D && height == 1) || textureType == ETextureType_2D);

	IdString idStr(szName);

	CTexture *pNewTexture = nullptr;
	{
		std::lock_guard<std::mutex> l(m_TextureMapLock);
		TextureMap::_MyIterType iter = m_TextureMap.Find(idStr);
		if (iter)
			return iter.Value();

		pNewTexture = CreateInstance(); //new (m_TexturePool.Allocate_mt()) CTexture;
		pNewTexture->m_IdStr = idStr;
		pNewTexture->m_nID = m_TextureId++;
		m_TextureMap.Insert(idStr, pNewTexture);
	}
	
	pNewTexture->Create(szName, width, height, textureType, pixelFormat, eUsage);
	return pNewTexture;
}

CTexture* CTextureManager::LoadTexture(const String &filePath, 
	EAutoGenmip bAutoGenMipmap /* = EAutoGenmip_AUTO */, bool bGamma /* = false */, bool bBackground /* = false */)
{
	IdString idStr(filePath);

	CTexture *pNewTexture = nullptr;
	{
		std::lock_guard<std::mutex> l(m_TextureMapLock);
		TextureMap::_MyIterType iter = m_TextureMap.Find(idStr);
		if (iter)
			return iter.Value();

		pNewTexture = CreateInstance(); //new (m_TexturePool.Allocate_mt()) CTexture;
		pNewTexture->m_IdStr = idStr;
		pNewTexture->m_nID = m_TextureId++;
		m_TextureMap.Insert(idStr, pNewTexture);
	}

	if (bBackground || !Global::IsMainThread()) {
		CJobSystem *pJobSystem = Global::m_pJobSystem;
	
		TextureLoadingJobData *pJobData = NEW_TYPE(TextureLoadingJobData);
		pJobData->szFileName = filePath;
		pJobData->bAutoGenMipmap = bAutoGenMipmap;
		pJobData->bGamma = bGamma;
		pJobData->pTexture = pNewTexture;

		Job *pJob = pJobSystem->CreateJob(EJobType::JobType_Texture);
		pJob->m_pData = (byte *)pJobData;
		pJobSystem->QueueJob(pJob);

		pNewTexture->m_pJob = pJob;
		
	}
	else {
		TEXTURE_FILE_DESC texture_desc;
		if (pNewTexture->Streaming(filePath, bAutoGenMipmap, &texture_desc))
			pNewTexture->Load(&texture_desc, bGamma);
		texture_desc.Release();
	}
	return pNewTexture;
}

void CTextureManager::DestroyTexture(CTexture *pTexture) {
	if (pTexture->CheckRefCount()) {
		// throw NEW_TYPE(std::exception)("Trying to destroy a texture while its reference count is not zero");
		assert(0 && "Trying to destroy a texture while its reference count is not zero");
		return;
	}

	bool bHas;
	{
		std::lock_guard<std::mutex> l(m_TextureMapLock);
		bHas = (bool)m_TextureMap.Find(pTexture->m_IdStr);
		if (bHas)
			m_TextureMap.Remove(pTexture->m_IdStr);
	}

	if (bHas)
		DestroyInstance(pTexture);
}

CTexture* CTextureManager::FindTexture(const String &szName) {
	IdString idStr(szName);
	std::lock_guard<std::mutex> l(m_TextureMapLock);
	TextureMap::_MyIterType iter = m_TextureMap.Find(idStr);
	return iter ? iter.Value() : nullptr;
}

CTexture* CTextureManager::FindTexture(const IdString &idStr) {
	std::lock_guard<std::mutex> l(m_TextureMapLock);
	TextureMap::_MyIterType iter = m_TextureMap.Find(idStr);
	return iter ? iter.Value() : nullptr;
}

CTexture* InternalTextures::s_pViewDepth = nullptr;