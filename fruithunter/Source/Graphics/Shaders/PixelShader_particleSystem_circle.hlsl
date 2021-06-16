struct GSOutput {
	float4 PosH : SV_POSITION;
	float4 Color : Color;
	float2 UV : UV;
};

float4 main(GSOutput input) : SV_TARGET {
	float4 outColor = input.Color;

	float2 center = float2(0.5f, 0.5f);
	float radius = length(input.UV - center);

	if (radius > 0.5f) {
		outColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
		clip(-1); // abort fragment rendering
	}

	return outColor;
}