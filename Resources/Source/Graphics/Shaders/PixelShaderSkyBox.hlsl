struct VS_OUT {
	float4 PosH : SV_POSITION;
	float2 TexCoord : TEXCOORD;
};
Texture2D oldTexture : TEXTURE : register(t1);
Texture2D newTexture : TEXTURE : register(t2);
SamplerState objSamplerState : SAMPLER : register(s0);
cbuffer deltaTime : register(b0) { float4 dt; };

float4 main(VS_OUT input) : SV_TARGET {
	float sampleAmount = dt.x;
	float3 color = (oldTexture.Sample(objSamplerState, input.TexCoord).rgb * (1 - sampleAmount)) +
				   (newTexture.Sample(objSamplerState, input.TexCoord).rgb * (sampleAmount));
	return float4(color, 1.0f);
}
