#include "Common.h"
#include "VertexLib.h"

float2 TextureDim;

VSOutputP4TC2 VS_FullScreenQuad(VSInputP2TC2 Input)
{
    VSOutputP4TC2 OutputP4TC2;
    POSITION_ELEM(OutputP4TC2) = float4(POSITION_ELEM(Input) + float2(-1.0f, 1.0f) / TextureDim, 0.0f, 1.0f);
    TEXCOORD_ELEM(OutputP4TC2, 0) = TEXCOORD_ELEM(Input, 0);
    return OutputP4TC2;
}