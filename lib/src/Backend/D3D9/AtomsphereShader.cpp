#include "AtomsphereRendererDX9.h"

String CAtomsphereRendererDX9::ShaderHeader() const
{
    return String(
        "const float2 Unit2 = { 1.0f, 1.0f };\n" \
        "const float3 Unit3 = { 1.0f, 1.0f, 1.0f };\n" \
        "const float3 Zero3 = { 0.0f, 0.0f, 0.0f };\n" \
        "float4 TopBottomRadius;\n" \

        "float ClampDistance(float d) {\n" \
        "return max(d, 0.0f);}\n" \

        "float SafeSqrt(float a) {\n" \
        "return sqrt(max(a, 0.0f));}\n" \

        "float3 DistanceToTopAtmosphereBoundary(float4 MU_R_Sqr){\n" \
        "float discriminant = MU_R_Sqr.w * (MU_R_Sqr.z - 1.0f) + TopBottomRadius.z;\n" \
        "return ClampDistance(-MU_R_Sqr.y * MU_R_Sqr.x + SafeSqrt(discriminant)).xxx;}\n" 
    );
}

String CAtomsphereRendererDX9::VertexShader() const
{
    return String(
        "struct VSOutput {\n" \
        "float4 Position : POSITION;\n" \
        "float2 Texcoord : TEXCOORD0;};\n" \
        "VSOutput DrawQuad(float2 Position : POSITION){\n" \
        "VSOutput o;\n" \
        "o.Position = float4(Position, 0.0f, 1.0f);\n" \
        "o.Texcoord = (Position * float2(1.0f, -1.0f) + float2(1.0f, 1.0f)) * 0.5f;\n" \
        "return o;}"
    );
}

String CAtomsphereRendererDX9::TransmittanceShader() const
{
    return String(
        "const int SAMPLE_COUNT = 500;\n"\
        "float2 TextureDim;\n" \

        "float3 RayleighScattering;\n" \
        "float3 MieExtinction;\n" \
        "float3 AbsorptionExtinction;\n" \

        "float3 RMAExpTerm[2];\n" \
        "float3 RMAExpScale[2];\n" \
        "float3 RMALinearTerm[2];\n" \
        "float3 RMAConstTerm[2];\n" \
        "float3 RMAWidth;\n" \

        "float3 GetLayerDensity(float3 altitude, int index){\n" \
        "float3 density = RMAExpTerm[index] * exp(RMAExpScale[index] * altitude) + RMALinearTerm[index] * altitude + RMAConstTerm[index];\n" \
        "return clamp(density, Zero3, Unit3);}\n" \

        "float3 GetProfileDensity(float3 altitude){\n" \
        "float3 ratio = clamp(sign(altitude - RMAWidth), Zero3, Unit3);\n" \
        "return lerp(GetLayerDensity(altitude, 0), GetLayerDensity(altitude, 1), ratio);}\n" \

        "float4 GetRMuFromTransmittanceTextureUv(float2 UV){\n" \
        "float4 MU_R_Sqr;\n" \
        "float2 X_MU_R = (UV - Unit2 / TextureDim * 0.5f) / (Unit2 - Unit2 / TextureDim);\n" \
        "float H = sqrt(TopBottomRadius.z - TopBottomRadius.w);\n" \
        "float rho = H * X_MU_R.y;\n" \
        "MU_R_Sqr.y = sqrt(rho * rho + TopBottomRadius.w);\n" \
        "float d_min = TopBottomRadius.x - MU_R_Sqr.y;\n" \
        "float d_max = rho + H;\n" \
        "float d = d_min + X_MU_R.x * (d_max - d_min);\n" \
        "MU_R_Sqr.x = d == 0.0f ? 1.0f : (H * H - rho * rho - d * d) / (2.0f * MU_R_Sqr.y * d);\n" \
        "MU_R_Sqr.zw = MU_R_Sqr.xy * MU_R_Sqr.xy;\n" \
        "return MU_R_Sqr;}\n" \

        "float3 ComputeOpticalLengthToTopAtmosphereBoundary(float4 MU_R_Sqr){\n" \
        "float3 dx = DistanceToTopAtmosphereBoundary(MU_R_Sqr) / SAMPLE_COUNT;\n" \
        "float3 result = Zero3;\n" \
        "for (int i = 0; i <= SAMPLE_COUNT; ++i){\n" \
        "float3 d_i = dx * i;\n" \
        "float3 r_i = sqrt(d_i * d_i + d_i * MU_R_Sqr.x * MU_R_Sqr.y * 2.0f + MU_R_Sqr.www);\n" \
        "float3 y_i = GetProfileDensity(r_i - TopBottomRadius.yyy);\n" \
        "result += y_i * dx * (i == 0 || i == SAMPLE_COUNT ? 0.5f : 1.0f);}\n" \
        "return result;}\n" \

        "float3 ComputeTransmittanceToTopAtmosphereBoundary(float4 MU_R_Sqr){\n" \
        "float3 OpticalLength = ComputeOpticalLengthToTopAtmosphereBoundary(MU_R_Sqr);\n" \
        "float3 Transmittance = RayleighScattering * OpticalLength.x + \n" \
        "MieExtinction * OpticalLength.y + \n" \
        "AbsorptionExtinction * OpticalLength.z;\n" \
        "return exp(-Transmittance);}\n" \

        "float4 ComputeTransmittanceToTopAtmosphereBoundaryTexture(float2 UV : TEXCOORD0) : COLOR0{\n" \
        "float4 MU_R_Sqr = GetRMuFromTransmittanceTextureUv(UV);\n" \
        "return float4(ComputeTransmittanceToTopAtmosphereBoundary(MU_R_Sqr), 1.0f);}\n"
    );
}

String CAtomsphereRendererDX9::IrradianceShader() const
{
    return String(
        "float4 TextureDim;\n" \
        "float SunAngularRadius;\n" \
        "sampler2D TransmittanceTexture;\n" \

        "float4 GetRMuSFromIrradianceTextureUv(float2 UV){\n" \
        "float4 MUS_R_Sqr;\n" \
        "float2 X_MUS_R = (UV - Unit2 / TextureDim.xy * 0.5f) / (Unit2 - Unit2 / TextureDim.xy);\n" \
        "MUS_R_Sqr.y = TopBottomRadius.y + X_MUS_R.y * (TopBottomRadius.x - TopBottomRadius.y);\n" \
        "MUS_R_Sqr.x = clamp(2.0f * X_MUS_R.x - 1.0f, -1.0f, 1.0f);\n" \
        "MUS_R_Sqr.zw = MUS_R_Sqr.xy * MUS_R_Sqr.xy;\n" \
        "return MUS_R_Sqr;}\n" \

        "float2 GetTransmittanceTextureUvFromRMu(float4 MUS_R_Sqr){\n" \
        "float H = sqrt(TopBottomRadius.z - TopBottomRadius.w);\n" \
        "float rho = SafeSqrt(MUS_R_Sqr.w - TopBottomRadius.w);\n" \
        "float3 d = DistanceToTopAtmosphereBoundary(MUS_R_Sqr);\n" \
        "float dMin = TopBottomRadius.x - MUS_R_Sqr.y;\n" \
        "float dMax = rho + H;\n" \
        "float2 X_MU_R = float2((d.x - dMin) / (dMax - dMin), rho / H);\n" \
        "return Unit2 * 0.5f / TextureDim.zw + X_MU_R * (Unit2 - (Unit2 / TextureDim.zw));}\n" \

        "float3 GetTransmittanceToTopAtmosphereBoundary(float4 MUS_R_Sqr){\n" \
        "float2 uv = GetTransmittanceTextureUvFromRMu(MUS_R_Sqr);\n" \
        "return tex2D(TransmittanceTexture, uv).xyz;}\n" \

        "float3 ComputeDirectIrradiance(float4 MUS_R_Sqr){\n" \
        "float Average_Cosine_Factor = MUS_R_Sqr.x < -SunAngularRadius ? 0.0f : (MUS_R_Sqr.x > SunAngularRadius ? MUS_R_Sqr.x :\n" \
        "(MUS_R_Sqr.x + SunAngularRadius) * (MUS_R_Sqr.x + SunAngularRadius) / (4.0f * SunAngularRadius));\n" \
        "return GetTransmittanceToTopAtmosphereBoundary(MUS_R_Sqr) * Average_Cosine_Factor * SunAngularRadius;}\n" \

        "float4 ComputeDirectIrradianceTexture(float2 UV : TEXCOORD0) : COLOR0{\n" \
        "float4 MUS_R_Sqr = GetRMuSFromIrrdianceTextureUv(UV);\n" \
        "return float4(ComputeDirectIrradiance(MUS_R_Sqr), 1.0f);}"
    );
}