struct GSOutput {
	float4 PosH : SV_POSITION;
	float4 Color : Color;
	float2 UV : UV;
};

Texture2D tex_particle : register(t0);

SamplerState samp;

float4 main(GSOutput input) : SV_TARGET {
	float4 outColor = tex_particle.Sample(samp, input.UV) * input.Color;
	clip(outColor.a - 0.01); // abort fragment if alpha lower than 0.01
	return outColor;
}