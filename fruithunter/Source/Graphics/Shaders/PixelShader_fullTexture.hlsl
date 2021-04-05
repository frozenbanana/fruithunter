struct Input {
	float4 posH : SV_POSITION;
	float2 uv : TexCoord;
};

Texture2D objTexture : TEXTURE : register(t0);
SamplerState objSamplerState : SAMPLER : register(s0);

float4 main(Input input) : SV_TARGET {
	float3 color = objTexture.Sample(objSamplerState, input.uv.xy).rgb;
	return float4(color, 1.f); // Updated 2
}
