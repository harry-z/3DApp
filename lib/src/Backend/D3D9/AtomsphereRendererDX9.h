#pragma once
#include "Atomsphere.h"
#include "../../RendererStableHeader.h"

#define CHECK_ERROR(hr) \
    if (FAILED(hr)) \
    { \
        Clean(); \
        return false; \
    }

#define CHECK_INDEX_ERROR(index) \
    if (index == 0xFFFFFFFF) \
    { \
        Clean(); \
        return false; \
    }

dword GetConstantIndex(ID3DXConstantTable *pConstTable, LPCSTR pszConstName);

class CTextureDX9;
class CAtomsphereRendererDX9 final : public IAtmosphereRenderer
{
public:
    CAtomsphereRendererDX9();
    virtual ~CAtomsphereRendererDX9();
    virtual bool Precompute(const AtmosphereParams &params) override;

private:
    void Clean();
    String ShaderHeader() const;
    String VertexShader() const;
    String TransmittanceShader() const;
    String IrradianceShader() const;
    String SingleScatteringShader() const;

    bool ProcessTransmittanceTexture(const String &szShaderHeader, const AtmosphereParams &params);
    bool ProcessIrradianceTexture(const String &szShaderHeader, const AtmosphereParams &params);
    bool ProcessSingleScatteringTexture(const String &szShaderHeader, const AtmosphereParams &params);

    double Interpolate(const CArray<double> &arrWaveLength, const CArray<double> &arrWaveLengthFunction, double WaveLength) const;

private:
    IDirect3DVertexShader9 *m_pVertexShader = nullptr;
    
    IDirect3DPixelShader9 *m_pTransmittanceShader = nullptr;
    ID3DXConstantTable *m_pTransmittanceShaderCode = nullptr;
    IDirect3DPixelShader9 *m_pIrradianceShader = nullptr;
    ID3DXConstantTable *m_pIrradianceShaderCode = nullptr;
    IDirect3DPixelShader9 *m_pSingleScatteringShader = nullptr;
    ID3DXConstantTable *m_pSingleScatteringShaderCode = nullptr;

    IDirect3DTexture9 *m_pTransmittance = nullptr;
    IDirect3DSurface9 *m_pTransmittanceSurface = nullptr;
    IDirect3DTexture9 *m_pIrradiance = nullptr;
    IDirect3DSurface9 *m_pIrradianceSurface = nullptr;

    IDirect3DVolumeTexture9 *m_pDeltaRayleighScattering = nullptr;
    IDirect3DSurface9 *m_ppDeltaRayleighScatteringSurface[SCATTERING_TEXTURE_DEPTH];
    IDirect3DVolumeTexture9 *m_pDeltaMieScattering = nullptr;
    IDirect3DSurface9 *m_ppDeltaMieScatteringSurface[SCATTERING_TEXTURE_DEPTH];
    IDirect3DVolumeTexture9 *m_pRayleighScattering = nullptr;
    IDirect3DSurface9 *m_ppRayleighScatteringSurface[SCATTERING_TEXTURE_DEPTH];
    IDirect3DVolumeTexture9 *m_pMieScattering = nullptr;
    IDirect3DSurface9 *m_ppMieScatteringSurface[SCATTERING_TEXTURE_DEPTH];

    IDirect3DSurface9 *m_pDSSurface = nullptr;

    IDirect3DVertexBuffer9 *m_pVB = nullptr;
    IDirect3DVertexDeclaration9 *m_pVDecl = nullptr;
};