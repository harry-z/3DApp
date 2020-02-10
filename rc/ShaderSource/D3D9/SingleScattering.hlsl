#include "Common.h"
#include "VertexLib.h"
#include "PixelLib.h"

float2 TextureDim;
float4 ScatteringTextureDimension; // NU_SIZE, MU_S_SIZE, MU_SIZE, R_SIZE
int Layer;
sampler2D TransmittanceTexture;

struct DeltaOutput {
    float3 DeltaRayleigh;
    float3 DeltaMie;
};

float4 GetRMuMuSNuFromScatteringTexture

DeltaOutput ComputeSingleScatteringTexture(float3 UVW)
{
    float4 RMuMuSNu = GetRMuMuSNuFromScatteringTexture()
}

PSMRTOutput4 PS_ComputeSingleScattering(VSOutputP4TC2 Input)
{
    PSMRTOutput4 MRTOutput4;
    DeltaOutput OutputColor = ComputeSingleScatteringTexture(float3(TEXCOORD_ELEM(Input, 0), Layer + 0.5f));
    MRTOutput4.Color0 = float4(OutputColor.DeltaRayleigh, 1.0f);
    MRTOutput4.Color1 = float4(OutputColor.DeltaMie, 1.0f);
    MRTOutput4.Color2 = float4(OutputColor.DeltaRayleigh, 1.0f);
    MRTOutput4.Color3 = float4(OutputColor.DeltaMie, 1.0f);
    return MRTOutput4;
}