#include "VertexLib.h"
#include "PixelLib.h"

VSOutputP4TC3 VS_SceneDepth(VSInputP3 Input)
{
	VSOutputP4TC3 OutputP4TC3;
	POSITION_ELEM(OutputP4TC3) = mul(WorldViewProjectionMatrix, float4(POSITION_ELEM(Input), 1.0f));
	TEXCOORD_ELEM(OutputP4TC3, 0) = mul(WorldViewMatrix, float4(POSITION_ELEM(Input), 1.0f)).xyz;
	return OutputP4TC3;		
}

PSSRTOutput PS_SceneDepth(VSOutputP4TC3 Input)
{
	PSSRTOutput SRTOutput;
	OUTPUT_CHANNEL(SRTOutput, 0) = float4(TEXCOORD_ELEM(Input, 0).z / (NearFarClip.y - NearFarClip.x), 0.0f, 0.0f, 1.0f);
	return SRTOutput;
}