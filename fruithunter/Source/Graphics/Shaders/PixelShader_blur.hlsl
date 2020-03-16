struct Input {
	float4 position : SV_POSITION;
	float3 color : COLOR;
	float2 uv : TEXCOORD;
};

Texture2D objTexture : TEXTURE : register(t0);
SamplerState objSamplerState : SAMPLER : register(s0);


float4 blur(float2 uv, float2 dir) {
	const float offset[] = { 0.0, 1.0, 2.0, 3.0, 4.0 };
	const float weight[] = { 0.2270270270, 0.1945945946, 0.1216216216, 0.0540540541, 0.0162162162 };
	float4 color = objTexture.Sample(objSamplerState, uv) * weight[0];

	float hstep, vstep;
	float2 fromNeighbour;
	for (int i = 1; i < 5; i++) {
		// Horizontal pass
		hstep = 1.0;
		vstep = 0.0;
		fromNeighbour = float2(hstep * offset[i], vstep * offset[i]);
		color += objTexture.Sample(objSamplerState, uv + fromNeighbour) * weight[i] +
				 objTexture.Sample(objSamplerState, uv - fromNeighbour) * weight[i];

		// Vertical pass
		hstep = 0.0;
		vstep = 1.0;
		fromNeighbour = float2(hstep * offset[i], vstep * offset[i]);
		color += objTexture.Sample(objSamplerState, uv + fromNeighbour) * weight[i] +
				 objTexture.Sample(objSamplerState, uv - fromNeighbour) * weight[i];
	}
	return color;
}

float4 main(Input input) : SV_TARGET {
	float4 color = blur(input.uv.xy, float2(1.0, 0.0));

	return color;
}
