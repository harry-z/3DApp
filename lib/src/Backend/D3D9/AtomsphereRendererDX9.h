#pragma once
#include "Atomsphere.h"
#include "../../RendererStableHeader.h"

class CTextureDX9;
class CAtomsphereRendererDX9 final : public IAtmosphereRenderer
{
public:
    virtual ~CAtomsphereRendererDX9();
    virtual bool Init(const AtmosphereParams &params) override;

private:
    String ShaderHeader(const AtmosphereParams &params) const;
    String TransmittanceShader() const;

    double Interpolate(const CArray<double> &arrWaveLength, const CArray<double> &arrWaveLengthFunction, double WaveLength) const;

private:
    IDirect3DTexture9 *m_pTransmittance = nullptr;
};