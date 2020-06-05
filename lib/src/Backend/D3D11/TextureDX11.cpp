#include "TextureDX11.h"
#include "PixelFormatUtil.h"
#include "RenderBackendDX11.h"

DXGI_FORMAT PixelFormatToDXGIFormat(EPixelFormat format) {
	switch (format) {
	case EPixelFormat::EPixelFormat_Unknown:
	default:
		return DXGI_FORMAT_UNKNOWN;
	case EPixelFormat::EPixelFormat_R9G9B9E5_SharedExp:
        return DXGI_FORMAT_R9G9B9E5_SHAREDEXP;
	case EPixelFormat::EPixelFormat_BC4:
        return DXGI_FORMAT_BC4_UNORM;
	case EPixelFormat::EPixelFormat_BC5:
        return DXGI_FORMAT_BC5_UNORM;
	case EPixelFormat::EPixelFormat_BC6H:
        return DXGI_FORMAT_BC6H_UF16;
	case EPixelFormat::EPixelFormat_BC7:
        return DXGI_FORMAT_BC7_UNORM;
	case EPixelFormat::EPixelFormat_L8:
		return DXGI_FORMAT_R8_UNORM;
	case EPixelFormat::EPixelFormat_A8:
		return DXGI_FORMAT_A8_UNORM;
	case EPixelFormat::EPixelFormat_A8L8:
		return DXGI_FORMAT_R8G8_UNORM;
	case EPixelFormat::EPixelFormat_X8R8G8B8:
	case EPixelFormat::EPixelFormat_R8G8B8:
		return DXGI_FORMAT_B8G8R8X8_UNORM;
	case EPixelFormat::EPixelFormat_A8R8G8B8:
		return DXGI_FORMAT_B8G8R8A8_UNORM;
	case EPixelFormat::EPixelFormat_A2R10G10B10:
		return DXGI_FORMAT_R10G10B10A2_UNORM;
	case EPixelFormat::EPixelFormat_F16_R:
		return DXGI_FORMAT_R16_FLOAT;
	case EPixelFormat::EPixelFormat_F16_RG:
		return DXGI_FORMAT_R16G16_FLOAT;
	case EPixelFormat::EPixelFormat_F16_RGBA:
		return DXGI_FORMAT_R16G16B16A16_FLOAT;
	case EPixelFormat::EPixelFormat_F32_R:
		return DXGI_FORMAT_R32_FLOAT;
	case EPixelFormat::EPixelFormat_F32_RG:
		return DXGI_FORMAT_R32G32_FLOAT;
	case EPixelFormat::EPixelFormat_F32_RGBA:
		return DXGI_FORMAT_R32G32B32A32_FLOAT;
	case EPixelFormat::EPixelFormat_Depth16:
		return DXGI_FORMAT_D16_UNORM;
	case EPixelFormat::EPixelFormat_Depth24:
		return DXGI_FORMAT_D24_UNORM_S8_UINT;
	case EPixelFormat::EPixelFormat_Depth32:
		return DXGI_FORMAT_D32_FLOAT;
	case EPixelFormat::EPixelFormat_BC1:
		return DXGI_FORMAT_BC1_UNORM;
	case EPixelFormat::EPixelFormat_BC2:
		return DXGI_FORMAT_BC2_UNORM;
	case EPixelFormat::EPixelFormat_BC3:
		return DXGI_FORMAT_BC3_UNORM;
	}
}

CTextureDX11::CTextureDX11() {
    m_Texture.m_pTexture1D = nullptr;
}

CTextureDX11::~CTextureDX11() {
	if (IsCreatedOrLoaded()) {
		ETextureType textureType = (ETextureType)(m_TexType & (~ETextureType_SRGB));
		switch (textureType) {
		case ETextureType_1D:
            m_Texture.m_pTexture1D->Release();
            break;
		case ETextureType_2D:
		case ETextureType_Cube:
			m_Texture.m_pTexture2D->Release();
			break;
		case ETextureType_3D:
			m_Texture.m_pTexture3D->Release();
			break;
		}
	}
	else if (m_pJob)
		m_pJob->Cancel();
}

void CTextureDX11::UpdateBuffer(const byte *pBuffer) {
	assert(BIT_CHECK(m_TexType, ETextureType_1D) || BIT_CHECK(m_TexType, ETextureType_2D));
	assert(m_Usage == (byte)ETextureUsage::ETextureUsage_Dynamic);

	if (IsCreatedOrLoaded())
	{
		if (m_TexType == ETextureType_1D)
		{
			D3D11_MAPPED_SUBRESOURCE MappedSubResource;
			if (SUCCEEDED(m_pDeviceContext11->Map(m_Texture.m_pTexture1D, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedSubResource)))
			{
				memcpy(MappedSubResource.pData, pBuffer, MappedSubResource.DepthPitch);
				m_pDeviceContext11->Unmap(m_Texture.m_pTexture1D, 0);
			}
		}
		else if (m_TexType == ETextureType_2D)
		{
			D3D11_MAPPED_SUBRESOURCE MappedSubResource;
			if (SUCCEEDED(m_pDeviceContext11->Map(m_Texture.m_pTexture2D, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedSubResource)))
			{
				memcpy(MappedSubResource.pData, pBuffer, MappedSubResource.DepthPitch);
				m_pDeviceContext11->Unmap(m_Texture.m_pTexture2D, 0);
			}
		}
	}
}

void CTextureDX11::Create(const String &sName, word nWidth, word nHeight, 
	ETextureType eTextureType, 
	EPixelFormat ePixelFormat, 
	ETextureUsage eTextureUsage) {

	HRESULT hr = E_FAIL;
	DXGI_FORMAT d3d_fmt = PixelFormatToDXGIFormat(ePixelFormat);
	if (d3d_fmt != DXGI_FORMAT_UNKNOWN) {
		if (eTextureType == ETextureType_1D) {
			nHeight = 1;
			if (eTextureUsage == ETextureUsage::ETextureUsage_Dynamic ||
				eTextureUsage == ETextureUsage::ETextureUsage_DynamicFixedSize) // 一维只允许创建Dynamic类型的纹理
            {
                D3D11_TEXTURE1D_DESC Desc;
                Desc.Width = nWidth;
                Desc.MipLevels = 1;
                Desc.ArraySize = 1;
                Desc.Format = d3d_fmt;
                Desc.Usage = D3D11_USAGE_DYNAMIC;
                Desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
                Desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
                Desc.MiscFlags = 0;
                hr = g_pDevice11->CreateTexture1D(&Desc, nullptr, &m_Texture.m_pTexture1D);
            }
		}
		else if (eTextureType == ETextureType_2D) {
            D3D11_TEXTURE2D_DESC Desc;
            Desc.Width = nWidth;
            Desc.Height = nHeight;
            Desc.MipLevels = 1;
            Desc.ArraySize = 1;
            Desc.Format = d3d_fmt;
            Desc.MiscFlags = 0;
            Desc.SamplerDesc.Count = 1;
            Desc.SamplerDesc.Quality = 0;
			if (eTextureUsage == ETextureUsage::ETextureUsage_Dynamic) // 创建Dynamic类型的二维纹理
            {
                Desc.Usage = D3D11_USAGE_DYNAMIC;
                Desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
                Desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
                hr = g_pDevice11->CreateTexture2D(&Desc, nullptr, &m_Texture.m_pTexture2D);
            }
			else if (eTextureUsage == ETextureUsage::ETextureUsage_RenderTarget || 
				eTextureUsage == ETextureUsage::ETextureUsage_RenderTargetFixedSize) { // 创建RenderTarget类型的二维纹理
                Desc.Usage = D3D11_USAGE_DEFAULT;
                Desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
                Desc.CPUAccessFlags = 0;
				hr = g_pDevice11->CreateTexture2D(&Desc, nullptr, &m_Texture.m_pTexture2D);
				if (SUCCEEDED(hr))
				{
					D3D11_RENDER_TARGET_VIEW_DESC RTViewDesc;
					RTViewDesc.Format = d3d_fmt;
					RTViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
					RTViewDesc.Texture2D.MipSlice = 0;
					hr = m_pDevice11->CreateRenderTargetView(m_Texture.m_pTexture2D, &RTViewDesc, &m_pRTView);
				}
				if (SUCCEEDED(hr))
				{
					D3D11_SHADER_RESOURCE_VIEW_DESC SRViewDesc;
					SRViewDesc.Format = d3d_fmt;
					SRViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
					SRViewDesc.Texture2D.MostDetailedMip = 0;
					SRViewDesc.Texture2D.MipLevels = 0;
					hr = m_pDevice11->CreateShaderResourceView(m_Texture.m_pTexture2D, &SRViewDesc, &m_pSRView);
				}
			}
			else if (eTextureUsage == ETextureUsage::ETextureUsage_DepthStencil ||
				eTextureUsage == ETextureUsage::ETextureUsage_DepthStencilFixedSize) { // 创建DepthStencil类型的二维纹理
				Desc.Usage = D3D11_USAGE_DEFAULT;
                Desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
                Desc.CPUAccessFlags = 0;
				hr = g_pDevice11->CreateTexture2D(&Desc, nullptr, &m_Texture.m_pTexture2D);
				if (SUCCEEDED(hr))
				{
					D3D11_DEPTH_STENCIL_VIEW_DESC DSViewDesc;
					DSViewDesc.Format = d3d_fmt;
					DSViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
					DSViewDesc.Flags = 0;
					DSViewDesc.Texture2D.MipSlice = 0;
					hr = m_pDevice11->CreateDepthStencilView(m_Texture.m_pTexture2D, &DSViewDesc, &m_pDSView);
				}
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
		m_CreatedOrLoaded = true;
	}
}

void CTextureDX11::Load(TEXTURE_FILE_DESC *pDesc, bool bGamma) {
	HRESULT hr = E_FAIL;
	DXGI_FORMAT d3d_fmt = PixelFormatToDXGIFormat(pDesc->eFormat);
	if (d3d_fmt != DXGI_FORMAT_UNKNOWN) {
		bool bNeedAutoMipmap = pDesc->eTexType != ETextureType_1D && pDesc->nMipLevel == 0;
		if (pDesc->eTexType == ETextureType_1D)
		{
			D3D11_TEXTURE1D_DESC Desc;
			Desc.Width = pDesc->nWidth;
			Desc.MipLevels = pDesc->nMipLevel;
			Desc.ArraySize = 1;
			Desc.Format = d3d_fmt;
			Desc.Usage = bNeedAutoMipMap ? D3D11_USAGE_DEFAULT : D3D11_USAGE_IMMUTABLE;
			Desc.BindFlags = bNeedAutoMipMap ? (D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET) : (D3D11_BIND_SHADER_RESOURCE);
			Desc.CPUAccessFlags = 0;
			Desc.MiscFlags = bNeedAutoMipmap ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0;
			D3D11_SUBRESOURCE_DATA SubRcData;
			SubRcData.pSysMem = pDesc->pData;
			SubRcData.SysMemPitch = SubRcData.SysMemSlicePitch = 0;
			hr = g_pDevice11->CreateTexture1D(&Desc, &SubRcData, &m_Texture.m_pTexture1D);
			if (SUCCEEDED(hr))
			{
				D3D11_SHADER_RESOURCE_VIEW_DESC SRViewDesc;
				SRViewDesc.Format = d3d_fmt;
				SRViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D;
				SRViewDesc.Texture1D.MostDetailedMip = 0;
				SRViewDesc.Texture1D.MipLevels = pDesc->nMipLevel;
				hr = g_pDevice11->CreateShaderResourceView(m_Texture.m_pTexture1D, &SRViewDesc, &m_pSRView);
			}
		}
		else if (pDesc->eTexType == ETextureType_2D)
		{
			D3D11_TEXTURE2D_DESC Desc;
			Desc.Width = pDesc->nWidth;
			Desc.Height = pDesc->nHeight;
			Desc.MipLevels = pDesc->nMipLevel;
			Desc.ArraySize = 1;
			Desc.Format = d3d_fmt;
			Desc.Usage = bNeedAutoMipMap ? D3D11_USAGE_DEFAULT : D3D11_USAGE_IMMUTABLE;
			Desc.BindFlags = bNeedAutoMipMap ? (D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET) : (D3D11_BIND_SHADER_RESOURCE);
			Desc.CPUAccessFlags = 0;
			Desc.MiscFlags = bNeedAutoMipmap ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0;
			Desc.SamplerDesc.Count = 1;
            Desc.SamplerDesc.Quality = 0;
			dword nNumBytes, nNumRowBytes, nNumRows;
			GetSurfaceInfo(pDesc->nWidth, pDesc->nHeight, pDesc->nDepth, pDesc->eFormat, &nNumBytes, &nNumRowBytes, nNumRows);
			D3D11_SUBRESOURCE_DATA SubRcData;
			SubRcData.pSysMem = pDesc->pData;
			SubRcData.SysMemPitch = nNumRowBytes;
			SubRcData.SysMemSlicePitch = 0;
			hr = g_pDevice11->CreateTexture2D(&Desc, &SubRcData, &m_Texture.m_pTexture2D);
			if (SUCCEEDED(hr))
			{
				D3D11_SHADER_RESOURCE_VIEW_DESC SRViewDesc;
				SRViewDesc.Format = d3d_fmt;
				SRViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
				SRViewDesc.Texture2D.MostDetailedMip = 0;
				SRViewDesc.Texture2D.MipLevels = pDesc->nMipLevel;
				hr = g_pDevice11->CreateShaderResourceView(m_Texture.m_pTexture2D, &SRViewDesc, &m_pSRView);
			}
		}
		else if (pDesc->eTexType == ETextureType_Cube)
		{
			D3D11_TEXTURE2D_DESC Desc;
			Desc.Width = pDesc->nWidth;
			Desc.Height = pDesc->nHeight;
			Desc.MipLevels = pDesc->nMipLevel;
			Desc.ArraySize = 6;
			Desc.Format = d3d_fmt;
			Desc.Usage = bNeedAutoMipMap ? D3D11_USAGE_DEFAULT : D3D11_USAGE_IMMUTABLE;
			Desc.BindFlags = bNeedAutoMipMap ? (D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET) : (D3D11_BIND_SHADER_RESOURCE);
			Desc.CPUAccessFlags = 0;
			Desc.MiscFlags = (bNeedAutoMipmap ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0) | D3D11_RESOURCE_MISC_TEXTURECUBE;
			Desc.SamplerDesc.Count = 1;
            Desc.SamplerDesc.Quality = 0;
			dword nNumBytes, nNumRowBytes, nNumRows;
			GetSurfaceInfo(pDesc->nWidth, pDesc->nHeight, pDesc->nDepth, pDesc->eFormat, &nNumBytes, &nNumRowBytes, nNumRows);
			D3D11_SUBRESOURCE_DATA SubRcData[6];
			for (int32 i = 0; i < 6; ++i)
			{
				memcpy(SubRcData[i].pSysMem, pDesc->pData + nNumRowBytes * nNumRows * i, nNumRowBytes * nNumRows);
				SubRcData[i].SysMemPitch = nNumRowBytes;
				SubRcData[i].SysMemSlicePitch = 0;
			}
			hr = g_pDevice11->CreateTexture2D(&Desc, &SubRcData[0], &m_Texture.m_pTexture2D);
			if (SUCCEEDED(hr))
			{
				D3D11_SHADER_RESOURCE_VIEW_DESC SRViewDesc;
				SRViewDesc.Format = d3d_fmt;
				SRViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
				SRViewDesc.TextureCube.MostDetailedMip = 0;
				SRViewDesc.TextureCube.MipLevels = pDesc->nMipLevel;
				hr = g_pDevice11->CreateShaderResourceView(m_Texture.m_pTexture2D, &SRViewDesc, &m_pSRView);
			}
		}
		else if (pDesc->eTexType == ETextureType_3D)
		{
			D3D11_TEXTURE3D_DESC Desc;
			Desc.Width = pDesc->nWidth;
			Desc.Height = pDesc->nHeight;
			Desc.Depth = pDesc->nDepth;
			Desc.MipLevels = pDesc->nMipLevel;
			Desc.ArraySize = 1;
			Desc.Format = d3d_fmt;
			Desc.Usage = bNeedAutoMipMap ? D3D11_USAGE_DEFAULT : D3D11_USAGE_IMMUTABLE;
			Desc.BindFlags = bNeedAutoMipMap ? (D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET) : (D3D11_BIND_SHADER_RESOURCE);
			Desc.CPUAccessFlags = 0;
			Desc.MiscFlags = (bNeedAutoMipmap ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0) | D3D11_RESOURCE_MISC_TEXTURECUBE;
			Desc.SamplerDesc.Count = 1;
            Desc.SamplerDesc.Quality = 0;
			dword nNumBytes, nNumRowBytes, nNumRows;
			GetSurfaceInfo(pDesc->nWidth, pDesc->nHeight, pDesc->nDepth, pDesc->eFormat, &nNumBytes, &nNumRowBytes, nNumRows);
			D3D11_SUBRESOURCE_DATA SubRcData;
			SubRcData.pSysMem = pDesc->pData;
			SubRcData.SysMemPitch = nNumRowBytes;
			SubRcData.SysMemSlicePitch = nNumRowBytes * nNumRows;
			hr = g_pDevice11->CreateTexture3D(&Desc, &SubRcData, &m_Texture.m_pTexture3D);
			if (SUCCEEDED(hr))
			{
				D3D11_SHADER_RESOURCE_VIEW_DESC SRViewDesc;
				SRViewDesc.Format = d3d_fmt;
				SRViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
				SRViewDesc.Texture3D.MostDetailedMip = 0;
				SRViewDesc.Texture3D.MipLevels = pDesc->nMipLevel;
				hr = g_pDevice11->CreateShaderResourceView(m_Texture.m_pTexture3D, &SRViewDesc, &m_pSRView);
			}
		}
	}
}

CTextureManagerDX11::CTextureManagerDX11() {
	m_TexturePool.Initialize(sizeof(CTextureDX11));
}

CTextureManagerDX11::~CTextureManagerDX11() {

}

bool CTextureManagerDX11::Initialize() {
	return true;
}

CTexture* CTextureManagerDX11::CreateInstance() {
	return new (m_TexturePool.Allocate_mt()) CTextureDX11;
}

void CTextureManagerDX11::DestroyInstance(CTexture *pTexture) {
	CTextureDX11 *pTextureDX11 = static_cast<CTextureDX11*>(pTexture);
	pTextureDX11->~CTextureDX11();
	m_TexturePool.Free_mt(pTextureDX11);
}