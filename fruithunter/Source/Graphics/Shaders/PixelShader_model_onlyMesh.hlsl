
struct PS_IN {
	float3 PosW : POSITION0;
	//float3 PosV : POSITION1;
	float4 PosH : SV_POSITION;
	float2 TexCoord : TEXCOORD;
	float3 Normal : NORMAL;
	float3 shadowPosH : POSITION2;
};

cbuffer colorBuffer : register(b2) { float4 color; }

float3 lighting(float3 pos, float3 normal, float3 color) {
	// LIGHTING
	// light
	float3 lightPos = float3(-5, 2, -3);
	float3 toLight = normalize(lightPos - pos);
	toLight = float3(1, 1, 1);
	// diffuse
	float shadowTint = max(dot(toLight, normal), 0.0);
	// specular
	float reflectTint =
		pow(max(dot(normalize(reflect(-toLight, normal)), normalize(-pos)), 0.0), 20.0);
	return color * (0.2 + shadowTint + reflectTint);
	//return color * (0.2 + shadowTint);
}

float4 main(PS_IN ip) : SV_TARGET {
	return float4(lighting(ip.PosW, ip.Normal.xyz, color.xyz), 1.0);
}

static const float SMAP_SIZE = 2048.0f;
static const float SMAP_DX = 1.0f / SMAP_SIZE;

float calcShadowFactor(SamplerComparisonState samShadow, Texture2D shadowMap, float4 shadowPosH) {
	// Divide to get coordinates in texture projection
	shadowPosH.xyz /= shadowPosH.w;

	// Depth in NDC
	float depth = shadowPosH.z;

	// Texel size
	const float dx = SMAP_DX;
	float percentLit = 0.0f;
	const float2 offsets[9] = { float2(-dx, -dx), float2(0.0f, -dx), float2(dx, -dx),
		float2(-dx, 0.0f), float2(0.0f, 0.0f), float2(dx, 0.0f), float2(-dx, +dx),
		float2(0.0f, +dx), float2(dx, +dx) };
	// 3x3 box filter pattern. Each sample does a 4-tap PCF.
	for (int i = 0; i < 9; i++) {
		percentLit += shadowMap.SampleCmpLevelZero(samShadow, shadowPosH.xy + offsets[i], depth).r;
	}
	return percentLit /= 9.0f;
}