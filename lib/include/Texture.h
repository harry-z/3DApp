#pragma once

#include "Job.h"
#include "Resource.h"
#include "TextureReader.h"



class DLL_EXPORT CTexture : public CBaseResource {
public:
	friend class CMaterial;

	inline word GetWidth() const { return m_nWidth; }
	inline word GetHeight() const { return m_nHeight; }
	inline byte GetDepthOrFace() const { return m_nDepth; }
	inline byte GetMipLevel() const { return m_nMipLevel; }
	inline EPixelFormat GetPixelFormat() const { return m_Format; }
	inline ETextureType GetTextureType() const { return (ETextureType)(m_TexType & (~ETextureType_SRGB)); }
	inline ETextureUsage GetTextureUsage() const { return (ETextureUsage)m_Usage; }
	inline bool NeedGammaCorrection() const { return BIT_CHECK(m_TexType, ((byte)ETextureType_SRGB)); }
	inline const IdString& GetIdString() const { return m_IdStr; }
	inline dword GetID() const { return m_nID; }

	virtual void UpdateBuffer(const byte *pBuffer) = 0;

protected:
	friend class CTextureManager;
	friend struct TextureLoadingJob;
	

	CTexture() :
		m_nWidth(0),
		m_nHeight(0),
		m_nDepth(0),
		m_nMipLevel(0),
		m_TexType(ETextureType_Unknown),
		m_Format(EPixelFormat::EPixelFormat_Unknown),
		m_Usage((byte)ETextureUsage::ETextureUsage_Unknown),
		m_pJob(nullptr) {}
	virtual ~CTexture() {}

	virtual void Create(const String &sName, word nWidth, word nHeight,
		ETextureType eTextureType,
		EPixelFormat ePixelFormat,
		ETextureUsage eTextureUsage) = 0;
	virtual void Load(TEXTURE_FILE_DESC *pDesc, bool bGamma) = 0;
	virtual void Destroy() override;

	bool Streaming(const String &sFilePath, EAutoGenmip bAutoGenMipmap, INOUT TEXTURE_FILE_DESC *pDesc);
	

protected:
	word m_nWidth;
	word m_nHeight;
	union {
		byte m_nDepth;
		byte m_nFace;
	};
	byte m_nMipLevel;
	byte m_TexType : 5;
	byte m_Usage : 3;
	EPixelFormat m_Format;

	dword m_nID;

	Job *m_pJob;
	IdString m_IdStr;
	
};

struct TextureLoadingJobData {
	String szFileName;
	CTexture *pTexture;
	EAutoGenmip bAutoGenMipmap;
	bool bGamma;
	TEXTURE_FILE_DESC texture_desc;

	~TextureLoadingJobData() {
		texture_desc.Release();
	}
};

struct TextureLoadingJob : public Job {
	// static const dword JobDataSize = sizeof(TextureLoadingJobData);
	static Job* CreateTextureLoadingJob(void *pMemory) { return new(pMemory) TextureLoadingJob; }
	static void DestroyTextureLoadingJob(Job *pJob) { ((TextureLoadingJob*)pJob)->~TextureLoadingJob(); }

	TextureLoadingJob() { m_Type = EJobType::JobType_Texture; }
	virtual void OnJobFailed() override;
	virtual void OnJobFinished() override;
	virtual bool DoWork_WorkerThread() override;
	virtual bool DoWork_MainThread() override;
};

REGISTER_JOB_CTOR_DTOR_DECL(TEXTURE_LOADING_JOB, EJobType::JobType_Texture, 
	TextureLoadingJob::CreateTextureLoadingJob, TextureLoadingJob::DestroyTextureLoadingJob);

class DLL_EXPORT CTextureManager {
public:
	CTextureManager();
	virtual ~CTextureManager() {}

	virtual bool Init() = 0;

	CTexture* CreateTexture(const String& szName, word width, word height,
		ETextureType textureType = ETextureType_2D,
		EPixelFormat pixelFormat = EPixelFormat::EPixelFormat_X8R8G8B8,
		ETextureUsage eUsage = ETextureUsage::ETextureUsage_Dynamic);
	CTexture* LoadTexture(const String &filePath, EAutoGenmip bAutoGenMipmap = EAutoGenmip::EAutoGenmip_AUTO, bool bGamma = false, bool bBackground = false);
	void DestroyTexture(CTexture *pTexture);
	CTexture* FindTexture(const String &szName);
	CTexture* FindTexture(const IdString &idStr);

protected:
	virtual CTexture* CreateInstance() = 0;
	virtual void DestroyInstance(CTexture *pTexture) = 0;
	
private:
	typedef CHashmap<IdString, CTexture*> TextureMap;
	TextureMap m_TextureMap;
	std::mutex m_TextureMapLock;
	dword m_TextureId;
};



