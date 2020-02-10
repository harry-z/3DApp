#include "Common.h"
#include "VertexLib.h"
#include "PixelLib.h"

const int SAMPLE_COUNT = 500;

// Transmittance texture dimension
float2 TextureDim;
// Atmosphere top radius, bottom radius, Top^2, Bottom^2
float4 TopBottomRadius; 
float3 RayleighScattering;
float3 MieExtinction;
float3 AbsorptionExtinction;
float3 RMAExpTerm[2];
float3 RMAExpScale[2];
float3 RMALinearTerm[2];
float3 RMAConstTerm[2];
float3 RMAWidth;

float3 DistanceToTopAtmosphereBoundary(float4 MU_R_Sqr)
{
    float discriminant = MU_R_Sqr.w * (MU_R_Sqr.z - 1.0f) + TopBottomRadius.z;
    return ClampDistance(-MU_R_Sqr.y * MU_R_Sqr.x + SafeSqrt(discriminant)).xxx;
}

float3 GetLayerDensity(float3 altitude, int index)
{
    float3 density = RMAExpTerm[index] * exp(RMAExpScale[index] * altitude) + RMALinearTerm[index] * altitude + RMAConstTerm[index];
    return clamp(density, Zero3, Unit3);
}

float3 GetProfileDensity(float3 altitude)
{
    float3 ratio = clamp(sign(altitude - RMAWidth), Zero3, Unit3);
    return lerp(GetLayerDensity(altitude, 0), GetLayerDensity(altitude, 1), ratio);
}

float4 GetRMuFromTransmittanceTextureUv(float2 UV)
{
    float4 MU_R_Sqr;
    // Shrink 0.5 texel
    float2 X_MU_R = (UV - Unit2 / TextureDim * 0.5f) / (Unit2 - Unit2 / TextureDim);
    // Mapping UV to MU and R
    float H = sqrt(TopBottomRadius.z - TopBottomRadius.w); 
    float rho = H * X_MU_R.y;
    // Calculate r (height)
    MU_R_Sqr.y = sqrt(rho * rho + TopBottomRadius.w);
    float d_min = TopBottomRadius.x - MU_R_Sqr.y;
    float d_max = rho + H;
    float d = lerp(d_min, d_max, X_MU_R.x);
    // Calculate cos(theta)
    MU_R_Sqr.x = clamp(d == 0.0f ? 1.0f : (H * H - rho * rho - d * d) / (2.0f * MU_R_Sqr.y * d), -1.0f, 1.0f);
    MU_R_Sqr.zw = MU_R_Sqr.xy * MU_R_Sqr.xy;
    return MU_R_Sqr;
}

float3 ComputeOpticalLengthToTopAtmosphereBoundary(float4 MU_R_Sqr)
{
    float3 dx = DistanceToTopAtmosphereBoundary(MU_R_Sqr) / SAMPLE_COUNT;
    float3 result = Zero3;
    for (int i = 0; i <= SAMPLE_COUNT; ++i)
    {
        float3 d_i = dx * i;
        // Calculate height at current sample point
        float3 r_i = sqrt(d_i * d_i + d_i * MU_R_Sqr.x * MU_R_Sqr.y * 2.0f + MU_R_Sqr.www);
        // Calculate density at current sample point
        float3 y_i = GetProfileDensity(r_i - TopBottomRadius.yyy);
        result += y_i * dx * (i == 0 || i == SAMPLE_COUNT ? 0.5f : 1.0f);
    }
    return result;
}

float3 ComputeTransmittanceToTopAtmosphereBoundary(float4 MU_R_Sqr)
{
    float3 OpticalLength = ComputeOpticalLengthToTopAtmosphereBoundary(MU_R_Sqr);
    float3 Transmittance = RayleighScattering * OpticalLength.xxx + 
        MieExtinction * OpticalLength.yyy + 
        AbsorptionExtinction * OpticalLength.zzz;
    return exp(-Transmittance);
}

PSSRTOutput PS_ComputeTransmittance(VSOutputP4TC2 Input)
{
    PSSRTOutput SRTOutput;
    float4 MU_R_Sqr = GetRMuFromTransmittanceTextureUv(TEXCOORD_ELEM(Input, 0));
    OUTPUT_CHANNEL(SRTOutput, 0) = float4(ComputeTransmittanceToTopAtmosphereBoundary(MU_R_Sqr), 1.0f);
    return SRTOutput;
}