struct Input {
	float4 position : SV_POSITION;
	float3 color : COLOR;
	float2 uv: TEXCOORD;
};

Texture2D objTexture : TEXTURE : register(t0);
SamplerState objSamplerState : SAMPLER : register(s0);

float4 main(Input input) : SV_TARGET
{
	float3 color = objTexture.Sample(objSamplerState, input.uv.xy);
	return float4(color, 1.0f);
}