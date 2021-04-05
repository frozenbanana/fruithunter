struct GSOutput {
	float4 PosH : SV_POSITION;
	float4 Color : Color;
	float2 UV : UV;
	float Size : Size;
};

#define NR_OF_LEAFS 3.000
#define LEAF_LENGTH 0.240
#define LEAF_SCALE 0.580

float circle(float2 pos, float radius) {
	float sph = length(pos) - radius;
	return smoothstep(radius, radius - 0.010f, sph);
}

float flower(float2 pos) {
	float a = atan(pos.y / pos.x);
	float r = length(pos) * 2.0f;
	float f = abs(cos(a * NR_OF_LEAFS / 2.f) * LEAF_SCALE) + LEAF_LENGTH;
	return 1.f - smoothstep(f, f + 0.028f, r);
}

float4 main(GSOutput input) : SV_TARGET {
	float4 outColor = float4(0.f, 0.f, 0.f, 0.f);
	float2 st = input.UV * 2. - 1.;
	float4 flowerColor = input.Color;
	float d = flower(st);
	if (d > 0.f) {
		outColor = d * flowerColor;
		outColor.a = 0.75;
	}
	/*d = circle(st, LEAF_SCALE / 12.f);
	if (d > 0.f) {
		outColor = d * float4(1.000, 0.490, 0.900, 0.75f);
	}*/
	return outColor;
}
