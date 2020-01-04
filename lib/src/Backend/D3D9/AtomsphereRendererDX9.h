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
    virtual ~CAtomsphereRendererDX9();
    virtual bool Precompute(const AtmosphereParams &params) override;

private:
    void Clean();
    String ShaderHeader() const;
    String VertexShader() const;
    String TransmittanceShader() const;
    String IrradianceShader() const;

    bool ProcessTransmittanceTexture(const String &szShaderHeader, const AtmosphereParams &params);
    bool ProcessIrradianceTexture(const String &szShaderHeader, const AtmosphereParams &params);

    double Interpolate(const CArray<double> &arrWaveLength, const CArray<double> &arrWaveLengthFunction, double WaveLength) const;

private:
    IDirect3DVertexShader9 *m_pVertexShader = nullptr;
    
    IDirect3DPixelShader9 *m_pTransmittanceShader = nullptr;
    ID3DXConstantTable *m_pTransmittanceShaderCode = nullptr;
    IDirect3DPixelShader9 *m_pIrradianceShader = nullptr;
    ID3DXConstantTable *m_pIrradianceShaderCode = nullptr;

    IDirect3DTexture9 *m_pTransmittance = nullptr;
    IDirect3DSurface9 *m_pTransmittanceSurface = nullptr;
    IDirect3DTexture9 *m_pIrradiance = nullptr;
    IDirect3DSurface9 *m_pIrradianceSurface = nullptr;

    IDirect3DSurface9 *m_pDSSurface = nullptr;

    IDirect3DVertexBuffer9 *m_pVB = nullptr;
    IDirect3DVertexDeclaration9 *m_pVDecl = nullptr;
};