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

    float Interpolate(const CArray<float> &arrWaveLength, const CArray<float> &arrWaveLengthFunction, float WaveLength) const;

private:
    IDirect3DTexture9 *m_pTransmittance = nullptr;
};