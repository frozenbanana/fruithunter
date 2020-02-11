struct VS_OUT {
	float4 PosH : SV_POSITION;
	float2 TexCoord : TEXCOORD;
};
Texture2D objTexture : TEXTURE : register(t1);
SamplerState objSamplerState : SAMPLER : register(s0);

float4 main(VS_OUT input) : SV_TARGET {
	float3 color = objTexture.Sample(objSamplerState, input.TexCoord).rgb;
	return float4(color, 1.0f);
	//return float4(1.0f, 0.0f, 0.0f, 1.0f);
}
