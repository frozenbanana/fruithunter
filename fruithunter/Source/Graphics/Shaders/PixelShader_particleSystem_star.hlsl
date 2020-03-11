struct GSOutput {
	float4 PosH : SV_POSITION;
	float4 Color : Color;
	float2 UV : UV;
	float Size : Size;
};
#define TWO_PI 6.283

float shape(float2 st, float ROT, int N) {
	// Angle and radius from the current pixel
	float a = atan(st.y / st.x) + ROT;
	float r = TWO_PI / float(N);

	// Shaping function that modulate the distance
	return cos(floor(.5 + a / r) * r - a) * length(st);
}

float4 main(GSOutput input) : SV_TARGET {

	// float4 outColor = input.Color;
	float4 outColor = float4(1.0, 0.0, 0.0, 1.0);
	float2 st = input.UV * 2. - 1.;

	int NR_OF_EDGES = 3;
	float ROT = 0.;
	float d = shape(st, ROT, NR_OF_EDGES);			  // triangle 1
	d = min(d, shape(st, ROT + 3.1415, NR_OF_EDGES)); // triangle 2

	outColor *= (1.0 - smoothstep(.4, .41, d));
	if (d < 0.01) {
		outColor.a = 0.f;
	}

	return outColor;
}