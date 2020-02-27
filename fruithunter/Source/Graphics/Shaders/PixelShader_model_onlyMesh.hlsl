
struct PS_IN {
	float3 PosW : POSITION0;
	//float3 PosV : POSITION1;
	float4 PosH : SV_POSITION;
	float2 TexCoord : TEXCOORD;
	float3 Normal : NORMAL;
	float4 ShadowPosH : POSITION2;
};

cbuffer colorBuffer : register(b2) { float4 color; }
Texture2D texture_shadowMap : register(t4);

SamplerState samplerAni {
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Linear;
	AddressV = Linear;
};

static const float SMAP_WIDTH = 2048.0f;
static const float SMAP_HEIGHT = 2048.0f;
static const float SMAP_DX = 1.0f / SMAP_WIDTH;
static const float SMAP_DY = 1.0f / SMAP_HEIGHT;

float calcShadowFactor(Texture2D shadowMap, float4 shadowPosH) {
	if (shadowPosH.x <= 0.0001f || shadowPosH.x >= 0.9999f) {
		// not a U coordinate
		return 1.0f;
	}
	if (shadowPosH.y <= 0.0001f || shadowPosH.y >= 0.9999f) {
		// not a U coordinate
		return 1.0f;
	}
	// Divide to get coordinates in texture projection
	// shadowPosH.xyz /= shadowPosH.w; //Not needed for orthographic

	// Depth in NDC
	float depth = shadowPosH.z - 0.005f;

	// Texel size
	const float dx = SMAP_DX;
	float percentLit = 0.0f;
	const float2 offsets[9] = { float2(-SMAP_DX, -SMAP_DY), float2(0.0f, -SMAP_DY),
		float2(SMAP_DX, -SMAP_DY), float2(-SMAP_DX, 0.0f), float2(0.0f, 0.0f),
		float2(SMAP_DX, 0.0f), float2(-SMAP_DX, +SMAP_DY), float2(0.0f, +SMAP_DY),
		float2(SMAP_DX, +SMAP_DY) };
	// 3x3 box filter pattern. Each sample does a 4-tap PCF.
	for (int i = 0; i < 9; i++) {
		percentLit += shadowMap.Sample(samplerAni, shadowPosH.xy + offsets[i]).r < depth;
	}
	percentLit /= 9.0f;
	return (1.0f - percentLit);
}

float3 lighting(float3 pos, float3 normal, float3 color, float shade) {
	// LIGHTING
	// light
	float3 lightPos = float3(-5, 2, -3);
	float3 toLight = normalize(lightPos - pos);
	toLight = float3(1, 1, 1);
	// diffuse
	float shadowTint = max(dot(toLight, normal), 0.0);
	// specular
	// float reflectTint =
	//	pow(max(dot(normalize(reflect(-toLight, normal)), normalize(-pos)), 0.0), 20.0);
	// return color * (0.2 + shadowTint + reflectTint);
	return color * (0.2 + shadowTint * shade);
}

float4 main(PS_IN ip) : SV_TARGET {
	float shade = calcShadowFactor(texture_shadowMap, ip.ShadowPosH);
	return float4(lighting(ip.PosW, ip.Normal.xyz, color.xyz, shade), 1.0);
}