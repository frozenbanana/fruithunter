
struct PS_IN {
	float3 PosW : POSITION0;
	// float3 PosV : POSITION1;
	float4 PosH : SV_POSITION;
	float2 TexCoord : TEXCOORD;
	float3 Normal : NORMAL;
	// float Height : LOCAL_Y;
	float4 ShadowPosH : POSITION2;
};

SamplerState samplerAni : register(s0);
Texture2D texture_aboveFlat : register(t0);
Texture2D texture_beneathFlat : register(t1);
Texture2D texture_aboveTilt : register(t2);
Texture2D texture_betweenTiltAndFlat : register(t3);
Texture2D texture_shadowMap : register(t4);

//SamplerComparisonState samShadow {
//	Filter = COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
//	AddressU = BORDER;
//	AddressV = BORDER;
//	AddressW = BORDER;
//	BorderColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
//	ComparionFunc = LESS_EQUAL;
//};

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

float specialLerp(float v, float min, float max) {
	return (v > max ? 1.f : (v < min ? 0.f : (v - min) / (max - min)));
}

static const float SMAP_WIDTH = 2048.0f;
static const float SMAP_HEIGHT = 2048.0f;
static const float SMAP_DX = 1.0f / SMAP_WIDTH;
static const float SMAP_DY = 1.0f / SMAP_HEIGHT;

float calcShadowFactor(Texture2D shadowMap, float4 shadowPosH) {
	if (shadowPosH.x < SMAP_DX*3.f || shadowPosH.x >= 1.0f - SMAP_DX*3.f) { 
		//not a U coordinate
		return 1.0f;
	}
	if (shadowPosH.y <= 0.0f || shadowPosH.y >= 1.0f) {
		// not a U coordinate
		return 1.0f;
	}
	// Divide to get coordinates in texture projection
	//shadowPosH.xyz /= shadowPosH.w; //Not needed for orthographic

	// Depth in NDC
	float depth = shadowPosH.z - 0.005f;

	// Texel size
	const float dx = SMAP_DX;
	float percentLit = 0.0f;
	const float2 offsets[9] = { 
		float2(-SMAP_DX, -SMAP_DY),	float2(0.0f, -SMAP_DY),	float2(SMAP_DX, -SMAP_DY),
		float2(-SMAP_DX, 0.0f),		float2(0.0f, 0.0f),		float2(SMAP_DX, 0.0f), 
		float2(-SMAP_DX, +SMAP_DY),	float2(0.0f, +SMAP_DY), float2(SMAP_DX, +SMAP_DY) };
	// 3x3 box filter pattern. Each sample does a 4-tap PCF.
	for (int i = 0; i < 9; i++) {
		percentLit += shadowMap.Sample(samplerAni, shadowPosH.xy + offsets[i]).r < depth;
	}
	percentLit /= 9.0f;
	return (1.0f - percentLit);
}

float4 main(PS_IN ip) : SV_TARGET {
	float3 aboveFlat = texture_aboveFlat.Sample(samplerAni, (ip.TexCoord * 50.)).rgb;
	float3 beneathFlat = texture_beneathFlat.Sample(samplerAni, (ip.TexCoord * 50.)).rgb;
	float3 aboveTilt = texture_aboveTilt.Sample(samplerAni, (ip.TexCoord * 50.f)).rgb;
	float3 betweenTiltAndFlat =
		texture_betweenTiltAndFlat.Sample(samplerAni, (ip.TexCoord * 50.)).rgb;
	// float3 beneathTilt = texture_beneathTilt.Sample(samplerAni, (ip.TexCoord * 50.) % 1.).rgb;

	float height = specialLerp(ip.PosW.y, 0.1f, 1.f);
	float dotN = dot(float3(0, 1, 0), ip.Normal);
	float tilt = specialLerp(dotN, 0.60f, 0.70f);

	float shade = calcShadowFactor(texture_shadowMap, ip.ShadowPosH);
	float3 flatColor = lerp(beneathFlat, aboveFlat, height);
	float3 baseTiltColor =
		lerp(aboveTilt, betweenTiltAndFlat, float3(1, 1, 1) * specialLerp(dotN, 0.45f, 0.55f));
	float3 baseColor =
		lerp(baseTiltColor, flatColor, float3(1, 1, 1) * specialLerp(dotN, 0.65f, 0.75f));

	return float4(lighting(ip.PosW, ip.Normal, baseColor, shade), 1.0);
	return float4(abs(ip.Normal), 1.0);
}