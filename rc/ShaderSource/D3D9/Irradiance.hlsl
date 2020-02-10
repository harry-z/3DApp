#include "Common.h"
#include "VertexLib.h"
#include "PixelLib.h"

// Irradiance texture and transmittance texture dimension
float4 TextureDim; 
// Atmosphere top radius, bottom radius, Top^2, Bottom^2
float4 TopBottomRadius;
float SunAngularRadius;

sampler2D TransmittanceTexture;

float3 DistanceToTopAtmosphereBoundary(float4 MU_R_Sqr)
{
    float discriminant = MU_R_Sqr.w * (MU_R_Sqr.z - 1.0f) + TopBottomRadius.z;
    return ClampDistance(-MU_R_Sqr.y * MU_R_Sqr.x + SafeSqrt(discriminant)).xxx;
}

float4 GetRMuSFromIrradianceTextureUv(float2 UV)
{
    float4 MUS_R_Sqr;
    float2 X_MUS_R = (UV - Unit2 / TextureDim.xy * 0.5f) / (Unit2 - Unit2 / TextureDim.xy);
    // Get height
    MUS_R_Sqr.y = TopBottomRadius.y + X_MUS_R.y * (TopBottomRadius.x - TopBottomRadius.y);
    // Get cos(theta)
	MUS_R_Sqr.x = clamp(2.0f * X_MUS_R.x - 1.0f, -1.0f, 1.0f);
    MUS_R_Sqr.zw = MUS_R_Sqr.xy * MUS_R_Sqr.xy;
    return MUS_R_Sqr;
}

float2 GetTransmittanceTextureUvFromRMu(float4 MUS_R_Sqr)
{
    float H = sqrt(TopBottomRadius.z - TopBottomRadius.w);
    float rho = SafeSqrt(MUS_R_Sqr.w - TopBottomRadius.w);
    float3 d = DistanceToTopAtmosphereBoundary(MUS_R_Sqr);
    float dMin = TopBottomRadius.x - MUS_R_Sqr.y;
    float dMax = rho + H;
    float2 X_MU_R = float2((d.x - dMin) / (dMax - dMin), rho / H);
    return Unit2 * 0.5f / TextureDim.zw + X_MU_R * (Unit2 - (Unit2 / TextureDim.zw));
}

float3 GetTransmittanceToTopAtmosphereBoundary(float4 MUS_R_Sqr)
{
    float2 uv = GetTransmittanceTextureUvFromRMu(MUS_R_Sqr);
    return tex2D(TransmittanceTexture, uv).xyz;
}

float3 ComputeDirectIrradiance(float4 MUS_R_Sqr)
{
    float Average_Cosine_Factor = MUS_R_Sqr.x < -SunAngularRadius ? 0.0f : (MUS_R_Sqr.x > SunAngularRadius ? MUS_R_Sqr.x :
		(MUS_R_Sqr.x + SunAngularRadius) * (MUS_R_Sqr.x + SunAngularRadius) / (4.0f * SunAngularRadius));
    return GetTransmittanceToTopAtmosphereBoundary(MUS_R_Sqr) * Average_Cosine_Factor * SunAngularRadius;
}

PSSRTOutput PS_ComputeDirectIrradiance(VSOutputP4TC2 Input)
{
    PSSRTOutput SRTOutput;
    float4 MUS_R_Sqr = GetRMuSFromIrradianceTextureUv(TEXCOORD_ELEM(Input, 0));
    OUTPUT_CHANNEL(SRTOutput, 0) = float4(ComputeDirectIrradiance(MUS_R_Sqr), 1.0f);
    return SRTOutput;
}