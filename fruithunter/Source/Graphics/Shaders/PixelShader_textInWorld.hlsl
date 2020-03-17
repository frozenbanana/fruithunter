
struct PS_IN {
	float3 PosW : POSITION0;
	// float3 PosV : POSITION1;
	float4 PosH : SV_POSITION;
	float2 TexCoord : TEXCOORD;
	float3 Normal : NORMAL;
	float4 ShadowPosH : POSITION2;
};

Texture2D textMap : register(t0); // text texture

SamplerState samplerAni;

float4 main(PS_IN ip) : SV_TARGET {
	// base color
	float4 pixelBaseColor = textMap.Sample(samplerAni, ip.TexCoord);

	return float4(pixelBaseColor.rgb, pixelBaseColor.a);
}