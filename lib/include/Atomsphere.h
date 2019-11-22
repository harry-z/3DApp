#pragma once
#include "Prereq.h"

constexpr int TRANSMITTANCE_TEXTURE_WIDTH = 256;
constexpr int TRANSMITTANCE_TEXTURE_HEIGHT = 64;

struct DensityProfileLayer
{
    DensityProfileLayer()
    : m_Width(0.0f)
    , m_ExpTerm(0.0f)
    , m_ExpScale(0.0f)
    , m_LinearTerm(0.0f)
    , m_ConstantTerm(0.0f)
    {}

    DensityProfileLayer(float Width, float ExpTerm, float ExpScale, float LinearTerm, float ConstantTerm)
    : m_Width(Width)
    , m_ExpTerm(ExpTerm)
    , m_ExpScale(ExpScale)
    , m_LinearTerm(LinearTerm)
    , m_ConstantTerm(ConstantTerm)
    {}

    float m_Width;
    float m_ExpTerm;
    float m_ExpScale;
    float m_LinearTerm;
    float m_ConstantTerm;
};

struct AtmosphereParams
{
    CArray<float> m_arrWaveLength;
    CArray<float> m_arrSolarIrradiance;
    CArray<DensityProfileLayer> m_arrRayleighDensity;
    CArray<float> m_arrRayleighScattering;
    CArray<DensityProfileLayer> m_arrMieDensity;
    CArray<float> m_arrMieScattering;
    CArray<float> m_arrMieExtinction;
    CArray<DensityProfileLayer> m_arrAbsorptionDensity;
    CArray<float> m_arrAbsorptionExtinction;
    CArray<float> m_arrGroundAlbedo;
    float m_SunAngularRadius;
    float m_BottomRadius;
    float m_TopRadius;
    float m_MiePhaseFunctionG;
    float m_MaxSunZenithAngle;
    float m_LengthUnitInMeters;
};

class IAtmosphereRenderer
{
public:
    virtual ~IAtmosphereRenderer() {}
    virtual bool Init(const AtmosphereParams &params) = 0;
};

class CAtmosphere
{
public:
    CAtmosphere();
    bool Init();

private:
    IAtmosphereRenderer *m_pRenderer;
};

