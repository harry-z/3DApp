#pragma once
#include "..\..\RendererStableHeader.h"
#include "Texture.h"
#include "GPUResource.h"

class CTextureDX9 final : public CTexture, public IGpuResource {
public:
    CTextureDX9();
    virtual ~CTextureDX9();
    virtual void UpdateBuffer(const byte *pBuffer) override;

protected:
    virtual void Create(const String &sName, word nWidth, word nHeight,
		ETextureType eTextureType,
		EPixelFormat ePixelFormat,
		ETextureUsage eTextureUsage) override;
	virtual void Load(TEXTURE_FILE_DESC *pDesc, bool bGamma) override;

    virtual void OnDeviceLost() override;
	virtual void OnDeviceReset(float w_changed_ratio, float h_changed_ratio) override;

public:
    union {
        LPDIRECT3DTEXTURE9 m_pTexture;
        LPDIRECT3DVOLUMETEXTURE9 m_pVolumeTexture;
        LPDIRECT3DCUBETEXTURE9 m_pCubeTexture;
    } m_Texture;

    LPDIRECT3DSURFACE9 m_pSurface = nullptr;
};

class CTextureManagerDX9 final : public CTextureManager {
public:
    CTextureManagerDX9();
    virtual ~CTextureManagerDX9();
    virtual bool Init() override;

protected:
	virtual CTexture* CreateInstance() override;
	virtual void DestroyInstance(CTexture *pTexture) override;

private:
    CPool m_TexturePool;
};