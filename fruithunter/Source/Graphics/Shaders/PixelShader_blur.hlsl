struct Input {
	float4 position : SV_POSITION;
	float3 color : COLOR;
	float2 uv : TEXCOORD;
};

Texture2D objTexture : TEXTURE : register(t0);
SamplerState objSamplerState : SAMPLER : register(s0);

float normpdf(float x, float sigma) {
	return 0.39894 * exp(-0.5 * x * x / (sigma * sigma)) / sigma;
}

float4 blur2(float2 uv) {
	// declare stuff
	const int mSize = 11;
	const int kSize = (mSize - 1) / 2;
	float kernel[mSize];
	float3 final_colour = float3(0.0, 0.0, 0.0);

	// create the 1-D kernel
	float sigma = 7.0;
	float Z = 0.0;
	for (int n = 0; n <= kSize; ++n) {
		kernel[kSize + n] = kernel[kSize - n] = normpdf(float(n), sigma);
	}

	// get the normalization factor (as the gaussian has been clamped)
	for (int j = 0; j < mSize; ++j) {
		Z += kernel[j];
	}

	// read out the texels
	for (int i = -kSize; i <= kSize; ++i) {
		for (int j = -kSize; j <= kSize; ++j) {
			final_colour +=
				kernel[kSize + j] * kernel[kSize + i] *
				objTexture.Sample(objSamplerState, uv + float2(float(i) / 1280.f, float(j) / 720.f))
					.rgb;
		}
	}


	return float4(final_colour / (Z * Z), 1.0);
}

float4 main(Input input) : SV_TARGET {
	float4 color = blur2(input.uv);

	return color;
}
