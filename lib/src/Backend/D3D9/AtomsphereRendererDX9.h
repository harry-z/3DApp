#pragma once
#include "Atomsphere.h"
#include "../../RendererStableHeader.h"

class CTextureDX9;
class CAtomsphereRendererDX9 final : public IAtmosphereRenderer
{
public:
    virtual ~CAtomsphereRendererDX9();
    virtual bool Precompute(
        const Vec4d &TopBottomRadius,
        const Vec3d &RayleighScattering,
        const Vec3d &MieExtinction,
        const Vec3d &AbsorptionExtinction,
        const CArray<DensityProfileLayer> &RayleighDensity,
        const CArray<DensityProfileLayer> &MieExtinctionDensity,
        const CArray<DensityProfileLayer> &AbsorptionExtinctionDensity) override;

private:
    void Clean();
    String ShaderHeader() const;
    String VertexShader() const;
    String TransmittanceShader() const;

    double Interpolate(const CArray<double> &arrWaveLength, const CArray<double> &arrWaveLengthFunction, double WaveLength) const;

private:
    IDirect3DVertexShader9 *m_pVertexShader = nullptr;
    IDirect3DPixelShader9 *m_pTransmittanceShader = nullptr;
    ID3DXConstantTable *m_pTransmittanceShaderCode = nullptr;
    IDirect3DTexture9 *m_pTransmittance = nullptr;
    IDirect3DSurface9 *m_pTransmittanceSurface = nullptr;
    IDirect3DSurface9 *m_pDSSurface = nullptr;
    IDirect3DVertexBuffer9 *m_pVB = nullptr;
    IDirect3DVertexDeclaration9 *m_pVDecl = nullptr;
};