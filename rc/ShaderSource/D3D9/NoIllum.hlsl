#include "VertexLib.h"
#include "PixelLib.h"
#include "ShadingModel.h"

VSOutputP4 VS_NoIllumColor(VSInputP3 Input) 
{
	VSOutputP4 OutputP4;
	POSITION_ELEM(OutputP4) = mul(WorldViewProjectionMatrix, float4(POSITION_ELEM(Input), 1.0f));
	return OutputP4;
}

VSOutputP4TC2 VS_NoIllum(VSInputP3TC2 Input)
{
	VSOutputP4TC2 OutputP4TC2;
	POSITION_ELEM(OutputP4TC2) = mul(WorldViewProjectionMatrix, float4(POSITION_ELEM(Input), 1.0f));
	TEXCOORD_ELEM(OutputP4TC2, 0) = TEXCOORD_ELEM(Input, 0);
	return OutputP4TC2;
}

float3 Albedo;
float Alpha;
sampler2D AlbedoTexture;

PSSRTOutput PS_NoIllumColor()
{
	PSSRTOutput SRTOutput;
	OUTPUT_CHANNEL(SRTOutput, 0) = NoIllumOpaque(Albedo);
	return SRTOutput;
}

PSSRTOutput PS_NoIllum(VSOutputP4TC2 Input)
{
	PSSRTOutput SRTOutput;
	OUTPUT_CHANNEL(SRTOutput, 0) = NoIllumOpaque(tex2D(AlbedoTexture, TEXCOORD_ELEM(Input, 0)).xyz);
	return SRTOutput;
}

PSSRTOutput PS_NoIllumBlended(VSOutputP4TC2 Input)
{
	PSSRTOutput SRTOutput;
	OUTPUT_CHANNEL(SRTOutput, 0) = NoIllumOpaque(tex2D(AlbedoTexture, TEXCOORD_ELEM(Input, 0)).xyz * Albedo);
	return SRTOutput;
}

PSSRTOutput PS_NoIllumColorTranslucent()
{
	PSSRTOutput SRTOutput;
	OUTPUT_CHANNEL(SRTOutput, 0) = NoIllum(Albedo, Alpha);
	return SRTOutput;
}

PSSRTOutput PS_NoIllumTranslucent(VSOutputP4TC2 Input)
{
	PSSRTOutput SRTOutput;
	float4 c = tex2D(AlbedoTexture, TEXCOORD_ELEM(Input, 0));
	OUTPUT_CHANNEL(SRTOutput, 0) = NoIllum(c.xyz, c.w * Alpha);
	return SRTOutput;
}

PSSRTOutput PS_NoIllumBlendedTranslucent(VSOutputP4TC2 Input)
{
	PSSRTOutput SRTOutput;
	float4 c = tex2D(AlbedoTexture, TEXCOORD_ELEM(Input, 0));
	OUTPUT_CHANNEL(SRTOutput, 0) = NoIllum(c.xyz * Albedo, c.w * Alpha);
	return SRTOutput;
}