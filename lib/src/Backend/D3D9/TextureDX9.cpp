#include "TextureDX9.h"
#include "PixelFormatUtil.h"
#include "RenderBackendDX9.h"

D3DFORMAT PixelFormatToD3DFORMAT(EPixelFormat format) {
	switch (format) {
	case EPixelFormat::EPixelFormat_Unknown:
	case EPixelFormat::EPixelFormat_R9G9B9E5_SharedExp:
	case EPixelFormat::EPixelFormat_BC4:
	case EPixelFormat::EPixelFormat_BC5:
	case EPixelFormat::EPixelFormat_BC6H:
	case EPixelFormat::EPixelFormat_BC7:
	default:
		return D3DFMT_UNKNOWN;
	case EPixelFormat::EPixelFormat_L8:
		return D3DFMT_L8;
	case EPixelFormat::EPixelFormat_A8:
		return D3DFMT_A8;
	case EPixelFormat::EPixelFormat_A8L8:
		return D3DFMT_A8L8;
	case EPixelFormat::EPixelFormat_X8R8G8B8:
	case EPixelFormat::EPixelFormat_R8G8B8:
		return D3DFMT_X8R8G8B8;
	case EPixelFormat::EPixelFormat_A8R8G8B8:
		return D3DFMT_A8R8G8B8;
	case EPixelFormat::EPixelFormat_A2R10G10B10:
		return D3DFMT_A2R10G10B10;
	case EPixelFormat::EPixelFormat_F16_R:
		return D3DFMT_R16F;
	case EPixelFormat::EPixelFormat_F16_RG:
		return D3DFMT_G16R16F;
	case EPixelFormat::EPixelFormat_F16_RGBA:
		return D3DFMT_A16B16G16R16F;
	case EPixelFormat::EPixelFormat_F32_R:
		return D3DFMT_R32F;
	case EPixelFormat::EPixelFormat_F32_RG:
		return D3DFMT_G32R32F;
	case EPixelFormat::EPixelFormat_F32_RGBA:
		return D3DFMT_A32B32G32R32F;
	case EPixelFormat::EPixelFormat_Depth16:
		return D3DFMT_D16;
	case EPixelFormat::EPixelFormat_Depth24:
		return D3DFMT_D24X8;
	case EPixelFormat::EPixelFormat_Depth32:
		return D3DFMT_D32;
	case EPixelFormat::EPixelFormat_BC1:
		return D3DFMT_DXT1;
	case EPixelFormat::EPixelFormat_BC2:
		return D3DFMT_DXT3;
	case EPixelFormat::EPixelFormat_BC3:
		return D3DFMT_DXT5;
	}
}

CTextureDX9::CTextureDX9() {
    m_Texture.m_pTexture = nullptr;
}

CTextureDX9::~CTextureDX9() {
	if (IsCreatedOrLoaded()) {
		if (m_Usage != (byte)ETextureUsage::ETextureUsage_Default) {
			((CRenderBackendDX9 *)Global::m_pRenderBackend)->m_lstGpuResource.Remove(&m_node);
		}
		SAFE_RELEASE(m_pSurface);
		ETextureType textureType = (ETextureType)(m_TexType & (~ETextureType_SRGB));
		switch (textureType) {
		case ETextureType_1D:
		case ETextureType_2D:
			m_Texture.m_pTexture->Release();
			break;
		case ETextureType_3D:
			m_Texture.m_pVolumeTexture->Release();
			break;
		case ETextureType_Cube:
			m_Texture.m_pCubeTexture->Release();
			break;
		}
	}
	else if (m_pJob)
		m_pJob->Cancel();
}

void CTextureDX9::UpdateBuffer(const byte *pBuffer) {
	assert(BIT_CHECK(m_TexType, ETextureType_1D) || BIT_CHECK(m_TexType, ETextureType_2D));
	assert(m_Usage == (byte)ETextureUsage::ETextureUsage_Dynamic);

	if (IsCreatedOrLoaded())
	{
		LPDIRECT3DTEXTURE9 pStagingTexture;
		g_pDevice9->CreateTexture(m_nWidth, m_nHeight, 1, 0, PixelFormatToD3DFORMAT(m_Format), D3DPOOL_SYSTEMMEM, &pStagingTexture, NULL);
		dword nBytes, nRowBytes, nRows;
		GetSurfaceInfo(m_nWidth, m_nHeight, 1, m_Format, &nBytes, &nRowBytes, &nRows);
		D3DLOCKED_RECT d3dlocked_rect;
		pStagingTexture->LockRect(0, &d3dlocked_rect, NULL, 0);
		memcpy(d3dlocked_rect.pBits, pBuffer, nBytes);
		pStagingTexture->UnlockRect(0);
		g_pDevice9->UpdateTexture(pStagingTexture, m_Texture.m_pTexture);
		SAFE_RELEASE(pStagingTexture);
	}
}

void CTextureDX9::Create(const String &sName, word nWidth, word nHeight, 
	ETextureType eTextureType, 
	EPixelFormat ePixelFormat, 
	ETextureUsage eTextureUsage) {

	HRESULT hr = E_FAIL;
	D3DFORMAT d3d_fmt = PixelFormatToD3DFORMAT(ePixelFormat);
	if (d3d_fmt != D3DFMT_UNKNOWN) {
		if (eTextureType == ETextureType_1D) {
			nHeight = 1;
			if (eTextureUsage == ETextureUsage::ETextureUsage_Dynamic ||
				eTextureUsage == ETextureUsage::ETextureUsage_DynamicFixedSize) // 一维只允许创建Dynamic类型的纹理
				hr = g_pDevice9->CreateTexture(nWidth, nHeight, 1, D3DUSAGE_DYNAMIC, d3d_fmt, D3DPOOL_DEFAULT, &m_Texture.m_pTexture, NULL);
		}
		else if (eTextureType == ETextureType_2D) {
			if (eTextureUsage == ETextureUsage::ETextureUsage_Dynamic) // 创建Dynamic类型的二维纹理
				hr = g_pDevice9->CreateTexture(nWidth, nHeight, 1, D3DUSAGE_DYNAMIC, d3d_fmt, D3DPOOL_DEFAULT, &m_Texture.m_pTexture, NULL);
			else if (eTextureUsage == ETextureUsage::ETextureUsage_RenderTarget || 
				eTextureUsage == ETextureUsage::ETextureUsage_RenderTargetFixedSize) { // 创建RenderTarget类型的二维纹理
				hr = g_pDevice9->CreateTexture(nWidth, nHeight, 1, D3DUSAGE_RENDERTARGET, d3d_fmt, D3DPOOL_DEFAULT, &m_Texture.m_pTexture, NULL);
				if (SUCCEEDED(hr))
					hr = m_Texture.m_pTexture->GetSurfaceLevel(0, &m_pSurface);
			}
			else if (eTextureUsage == ETextureUsage::ETextureUsage_DepthStencil ||
				eTextureUsage == ETextureUsage::ETextureUsage_DepthStencilFixedSize) { // 创建DepthStencil类型的二维纹理
				hr = g_pDevice9->CreateTexture(nWidth, nHeight, 1, D3DUSAGE_DEPTHSTENCIL, d3d_fmt, D3DPOOL_DEFAULT, &m_Texture.m_pTexture, NULL);
				if (SUCCEEDED(hr))
					hr = m_Texture.m_pTexture->GetSurfaceLevel(0, &m_pSurface);
			}
		}
	}
	
	if (SUCCEEDED(hr)) {
		m_nWidth = nWidth;
		m_nHeight = nHeight;
		m_nMipLevel = 1;
		m_Format = ePixelFormat;
		m_TexType = eTextureType;
		m_Usage = (byte)eTextureUsage;
		((CRenderBackendDX9 *)Global::m_pRenderBackend)->m_lstGpuResource.PushBack(&m_node);
		m_CreatedOrLoaded = true;
	}
}

void CTextureDX9::Load(TEXTURE_FILE_DESC *pDesc, bool bGamma) {
	HRESULT hr = E_FAIL;
	D3DFORMAT d3d_fmt = PixelFormatToD3DFORMAT(pDesc->eFormat);
	if (d3d_fmt != D3DFMT_UNKNOWN) {
		DWORD dwUsage = 0;
		if (pDesc->eTexType != ETextureType_1D) { // 一维纹理不支持mipmap
			if (pDesc->nMipLevel == 0) { // 当MipLevel是0时需要检查是否能够自动生成mipmap
				if (g_pCaps9->Caps2 & D3DCAPS2_CANAUTOGENMIPMAP)
					dwUsage = D3DUSAGE_AUTOGENMIPMAP;
				else
					pDesc->nMipLevel = 1;
			}
		}

		if (pDesc->eTexType == ETextureType_1D || pDesc->eTexType == ETextureType_2D)
			hr = g_pDevice9->CreateTexture(pDesc->nWidth, pDesc->nHeight, pDesc->nMipLevel, dwUsage, d3d_fmt, D3DPOOL_MANAGED, &m_Texture.m_pTexture, NULL);
		else if (pDesc->eTexType == ETextureType_3D)
			hr = g_pDevice9->CreateVolumeTexture(pDesc->nWidth, pDesc->nHeight, pDesc->nDepth, pDesc->nMipLevel, dwUsage, d3d_fmt, D3DPOOL_MANAGED, &m_Texture.m_pVolumeTexture, NULL);
		else if (pDesc->eTexType == ETextureType_Cube)
			hr = g_pDevice9->CreateCubeTexture(pDesc->nWidth, pDesc->nMipLevel, dwUsage, d3d_fmt, D3DPOOL_MANAGED, &m_Texture.m_pCubeTexture, NULL);

		if (SUCCEEDED(hr)) {
			if (pDesc->eTexType == ETextureType_1D || pDesc->eTexType == ETextureType_2D) {
				dword nBytes, nRowBytes, nRows;
				D3DLOCKED_RECT d3dlocked_rect;
				if (dwUsage & D3DUSAGE_AUTOGENMIPMAP)
				{
					GetSurfaceInfo(pDesc->nWidth, pDesc->nHeight, 1, pDesc->eFormat, &nBytes, &nRowBytes, &nRows);
					m_Texture.m_pTexture->LockRect(0, &d3dlocked_rect, NULL, 0);
					memcpy(d3dlocked_rect.pBits, pDesc->pData, nBytes);
					m_Texture.m_pTexture->UnlockRect(0);
					m_Texture.m_pTexture->GenerateMipSubLevels();
				}
				else
				{
					byte *pSrcTemp = (byte*)pDesc->pData;
					word w = pDesc->nWidth, h = pDesc->nHeight;
					LPDIRECT3DSURFACE9 pDestSurface;
					for (byte i = 0; i < pDesc->nMipLevel; ++i)
					{
						GetSurfaceInfo(w, h, 1, pDesc->eFormat, &nBytes, &nRowBytes, &nRows);
						m_Texture.m_pTexture->GetSurfaceLevel(i, &pDestSurface);
						pDestSurface->LockRect(&d3dlocked_rect, NULL, 0);
						memcpy(d3dlocked_rect.pBits, pSrcTemp, nBytes);
						pDestSurface->UnlockRect();

						w = w >> 1;
						h = h >> 1;
						if (w == 0)
							w = 1;
						if (h == 0)
							h = 1;
						pSrcTemp += nBytes;
						SAFE_RELEASE(pDestSurface);
					}
				}
			}
			else if (pDesc->eTexType == ETextureType_3D) {
				dword nBytes, nRowBytes, nRows;
				D3DLOCKED_BOX d3dlocked_box;
				if (dwUsage & D3DUSAGE_AUTOGENMIPMAP)
				{
					GetSurfaceInfo(pDesc->nWidth, pDesc->nHeight, pDesc->nDepth, pDesc->eFormat, &nBytes, &nRowBytes, &nRows);
					m_Texture.m_pVolumeTexture->LockBox(0, &d3dlocked_box, NULL, 0);
					memcpy(d3dlocked_box.pBits, pDesc->pData, nBytes);
					m_Texture.m_pVolumeTexture->UnlockBox(0);
					m_Texture.m_pVolumeTexture->GenerateMipSubLevels();
				}
				else
				{
					byte *pSrcTemp = (byte*)pDesc->pData;
					word w = pDesc->nWidth, h = pDesc->nHeight;
					byte d = pDesc->nDepth;
					LPDIRECT3DVOLUME9 pDestVolume;
					for (byte i = 0; i < pDesc->nMipLevel; ++i)
					{
						GetSurfaceInfo(w, h, d, pDesc->eFormat, &nBytes, &nRowBytes, &nRows);
						m_Texture.m_pVolumeTexture->GetVolumeLevel(i, &pDestVolume);
						pDestVolume->LockBox(&d3dlocked_box, NULL, 0);
						memcpy(d3dlocked_box.pBits, pSrcTemp, nBytes);
						pDestVolume->UnlockBox();

						w = w >> 1;
						h = h >> 1;
						d = d >> 1;
						if (w == 0)
							w = 1;
						if (h == 0)
							h = 1;
						if (d == 0)
							d = 1;
						pSrcTemp += nBytes;
						SAFE_RELEASE(pDestVolume);
					}
				}
			}
			else if (pDesc->eTexType == ETextureType_Cube) {
				dword nBytes, nRowBytes, nRows;
				D3DLOCKED_RECT d3dlocked_rect;
				byte *p = (byte*)pDesc->pData;
				if (dwUsage & D3DUSAGE_AUTOGENMIPMAP)
				{
					GetSurfaceInfo(pDesc->nWidth, pDesc->nWidth, 1, pDesc->eFormat, &nBytes, &nRowBytes, &nRows);
					byte nFlag = ECubeFace_PosX;
					// 参见DXSDK "DDS Cube Map Example"
					for (byte i = 0; i < 6; ++i)
					{
						if (pDesc->nFace & nFlag)
						{
							m_Texture.m_pCubeTexture->LockRect((D3DCUBEMAP_FACES)i, 0, &d3dlocked_rect, NULL, 0);
							memcpy(d3dlocked_rect.pBits, p, nBytes);
							m_Texture.m_pCubeTexture->UnlockRect((D3DCUBEMAP_FACES)i, 0);
							p += nBytes;
						}
						nFlag = nFlag << 1;
					}
					m_Texture.m_pCubeTexture->GenerateMipSubLevels();
				}
				else
				{
					word nedge = pDesc->nWidth;
					byte nFlag = ECubeFace_PosX;
					// 参见DXSDK "DDS Cube Map Example"
					for (byte i = 0; i < 6; ++i)
					{
						if (pDesc->nFace & nFlag)
						{
							for (byte j = 0; j < pDesc->nMipLevel; ++j)
							{
								GetSurfaceInfo(nedge, nedge, 1, pDesc->eFormat, &nBytes, &nRowBytes, &nRows);
								m_Texture.m_pCubeTexture->LockRect((D3DCUBEMAP_FACES)i, j, &d3dlocked_rect, NULL, 0);
								memcpy(d3dlocked_rect.pBits, p, nBytes);
								m_Texture.m_pCubeTexture->UnlockRect((D3DCUBEMAP_FACES)i, j);

								nedge = nedge >> 1;
								if (nedge == 0)
									nedge = 1;

								p += nBytes;
							}
							nedge = pDesc->nWidth;
						}
						nFlag = nFlag << 1;
					}
				}
			}
		}
	}

	if (SUCCEEDED(hr)) {
		m_nWidth = pDesc->nWidth;
		m_nHeight = pDesc->nHeight;
		m_nDepth = pDesc->nDepth;
		m_nMipLevel = pDesc->nMipLevel;
		m_Format = pDesc->eFormat;
		m_TexType = pDesc->eTexType | (bGamma ? ETextureType_SRGB : 0);
		m_Usage = (byte)ETextureUsage::ETextureUsage_Default;
		m_CreatedOrLoaded = 1;
	}
}

void CTextureDX9::OnDeviceLost() {
	switch (GetTextureType()) {
		case ETextureType_1D:
		case ETextureType_2D:
		{
			SAFE_RELEASE(m_Texture.m_pTexture);
			break;
		}
		case ETextureType_3D:
		{
			SAFE_RELEASE(m_Texture.m_pVolumeTexture);
			break;
		}
		case ETextureType_Cube:
		{
			SAFE_RELEASE(m_Texture.m_pCubeTexture);
			break;
		}
	}
	SAFE_RELEASE(m_pSurface);
}

void CTextureDX9::OnDeviceReset(float w_changed_ratio, float h_changed_ratio) {
	DWORD dwUsage = 0;
	switch (GetTextureUsage())
	{
	case ETextureUsage::ETextureUsage_Dynamic:
		dwUsage = D3DUSAGE_DYNAMIC;
		m_nWidth = m_nWidth * w_changed_ratio;
		m_nHeight = m_nHeight * h_changed_ratio;
		break;
	case ETextureUsage::ETextureUsage_DynamicFixedSize:
		dwUsage = D3DUSAGE_DYNAMIC;
		break;
	case ETextureUsage::ETextureUsage_RenderTarget:
		dwUsage = D3DUSAGE_RENDERTARGET;
		m_nWidth = m_nWidth * w_changed_ratio;
		m_nHeight = m_nHeight * h_changed_ratio;
		break;
	case ETextureUsage::ETextureUsage_RenderTargetFixedSize:
		dwUsage = D3DUSAGE_RENDERTARGET;
		break;
	case ETextureUsage::ETextureUsage_DepthStencil:
		dwUsage = D3DUSAGE_DEPTHSTENCIL;
		m_nWidth = m_nWidth * w_changed_ratio;
		m_nHeight = m_nHeight * h_changed_ratio;
		break;
	case ETextureUsage::ETextureUsage_DepthStencilFixedSize:
		dwUsage = D3DUSAGE_DEPTHSTENCIL;
		break;
	}
	HRESULT hr = g_pDevice9->CreateTexture(m_nWidth, m_nHeight, m_nMipLevel,
		dwUsage, PixelFormatToD3DFORMAT(m_Format), D3DPOOL_DEFAULT, &m_Texture.m_pTexture, NULL);
	assert(SUCCEEDED(hr) && "Error when recreating texture during device reset");
	hr = m_Texture.m_pTexture->GetSurfaceLevel(0, &m_pSurface);
	assert(SUCCEEDED(hr) && "Error when getting texture surface during device reset");
}

CTextureManagerDX9::CTextureManagerDX9() {
	m_TexturePool.Initialize(sizeof(CTextureDX9));
}

CTextureManagerDX9::~CTextureManagerDX9() {
	InternalTextures::s_pViewDepth.Detach();
}

bool CTextureManagerDX9::Initialize() {
	bool bRet = true;

	dword w, h;
	Global::m_pDisplay->GetClientDimension(w, h);
	InternalTextures::s_pViewDepth = CreateTexture("SceneDepth", w, h, ETextureType_2D, EPixelFormat::EPixelFormat_F16_R, ETextureUsage::ETextureUsage_RenderTarget);
	bRet |= InternalTextures::s_pViewDepth->IsCreatedOrLoaded();

	return bRet;
}

CTexture* CTextureManagerDX9::CreateInstance() {
	return new (m_TexturePool.Allocate_mt()) CTextureDX9;
}

void CTextureManagerDX9::DestroyInstance(CTexture *pTexture) {
	CTextureDX9 *pTextureDX9 = static_cast<CTextureDX9*>(pTexture);
	pTextureDX9->~CTextureDX9();
	m_TexturePool.Free_mt(pTextureDX9);
}

void CRenderBackendDX9::SetTarget(dword nNumColorBuffers, CTexture **ppColorBuffer, CTexture *pDepthStencilBuffer) {
	for (dword i = 0; i < nNumColorBuffers; ++i)
	{
		CTextureDX9 *pRenderTarget9 = (CTextureDX9 *)ppColorBuffer[i];
		assert(pRenderTarget9->GetTextureUsage() == ETextureUsage::ETextureUsage_RenderTarget ||
			pRenderTarget9->GetTextureUsage() == ETextureUsage::ETextureUsage_RenderTargetFixedSize);
		m_pD3DDevice9->SetRenderTarget(i, pRenderTarget9->m_pSurface);
	}

	if (pDepthStencilBuffer) {
		CTextureDX9 *pDepthStencil9 = (CTextureDX9 *)pDepthStencilBuffer;
		assert(pDepthStencil9->GetTextureUsage() == ETextureUsage::ETextureUsage_DepthStencil || 
			pDepthStencil9->GetTextureUsage() == ETextureUsage::ETextureUsage_DepthStencilFixedSize);
		m_pD3DDevice9->SetDepthStencilSurface(pDepthStencil9->m_pSurface);
	}
	
	D3DVIEWPORT9 viewport;
	ZeroMemory(&viewport, sizeof(D3DVIEWPORT9));
	viewport.Width = ppColorBuffer[0]->GetWidth();
	viewport.Height = ppColorBuffer[0]->GetHeight();
	viewport.MaxZ = 1.0f;
	m_pD3DDevice9->SetViewport(&viewport);
}

void CRenderBackendDX9::SetTexture(dword nSlot, CTexture *pTexture)
{
	m_pD3DDevice9->SetSamplerState(nSlot, D3DSAMP_SRGBTEXTURE, pTexture->NeedGammaCorrection() ? 1 : 0);

	switch (pTexture->GetTextureType())
	{
		case ETextureType_1D:
		case ETextureType_2D:
		{
			CTextureDX9 *pTextureDX9 = static_cast<CTextureDX9*>(pTexture);
			m_pD3DDevice9->SetTexture(nSlot, pTextureDX9->m_Texture.m_pTexture);
			break;
		}
		case ETextureType_3D:
		{
			CTextureDX9 *pTextureDX9 = static_cast<CTextureDX9*>(pTexture);
			m_pD3DDevice9->SetTexture(nSlot, pTextureDX9->m_Texture.m_pVolumeTexture);
			break;
		}
		case ETextureType_Cube:
		{
			CTextureDX9 *pTextureDX9 = static_cast<CTextureDX9*>(pTexture);
			m_pD3DDevice9->SetTexture(nSlot, pTextureDX9->m_Texture.m_pCubeTexture);
			break;
		}
	}
}