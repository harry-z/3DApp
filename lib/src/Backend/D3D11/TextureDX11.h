#pragma once
#include "..\..\RendererStableHeader.h"
#include "Texture.h"

class CTextureDX11 final : public CTexture
{
public:
    CTextureDX11();
    virtual ~CTextureDX11();
    virtual void UpdateBuffer(const byte *pBuffer) override;

protected:
    virtual void Create(const String &sName, word nWidth, word nHeight,
		ETextureType eTextureType,
		EPixelFormat ePixelFormat,
		ETextureUsage eTextureUsage) override;
	virtual void Load(TEXTURE_FILE_DESC *pDesc, bool bGamma) override;

public:
    union {
        ID3D11Texture1D *m_pTexture1D;
        ID3D11Texture2D *m_pTexture2D;
        ID3D11Texture3D *m_pTexture3D;
    } m_Texture;

    ID3D11ShaderResourceView *m_pSRView = nullptr;
    ID3D11RenderTargetView *m_pRTView = nullptr;
    ID3D11DepthStencilView *m_pDSView = nullptr;
};

class CTextureManagerDX11 final : public CTextureManager {
public:
    CTextureManagerDX11();
    virtual ~CTextureManagerDX11();
    virtual bool Initialize() override;

protected:
	virtual CTexture* CreateInstance() override;
	virtual void DestroyInstance(CTexture *pTexture) override;

private:
    CPool m_TexturePool;
};