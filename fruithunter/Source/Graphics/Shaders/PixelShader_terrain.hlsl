
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
cbuffer lightInfo : register(b5) {
	float4 ambient;
	float4 diffuse;
	float4 specular;
};
SamplerComparisonState samShadow { 
	Filter = COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
	AddressU = BORDER;
	AddressV = BORDER;
	AddressW = BORDER;
	BorderColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
	ComparionFunc = LESS_EQUAL;
};

float3 lighting(float3 pos, float3 normal, float3 color, float shade) {
	// light utility
	float3 lightPos = float3(0.f, 110.f, 100.f);
	float3 toLight = normalize(lightPos - pos);

	// diffuse
	float shadowTint = max(dot(toLight, normal), 0.0);

	// specular
	// float reflectTint =
	//	pow(max(dot(normalize(reflect(-toLight, normal)), normalize(-pos)), 0.0), 20.0);
	// return color * (0.2 + shadowTint + reflectTint);
	return color * (0.2 * ambient + shadowTint * shade * diffuse);
}

float specialLerp(float v, float min, float max) {
	return (v > max ? 1.f : (v < min ? 0.f : (v - min) / (max - min)));
}

float random(float3 seed, int i) {
	float4 seed4 = float4(seed, i);
	float dot_product = dot(seed4, float4(12.9898, 78.233, 45.164, 94.673));
	return frac(sin(dot_product) * 43758.5453);
}

float2 poissonDisk[16] = {float2(-0.94201624, -0.39906216), float2(0.94558609, -0.76890725),
	float2(-0.094184101, -0.92938870), float2(0.34495938, 0.29387760),
	float2(-0.91588581, 0.45771432),
	float2(-0.81544232, -0.87912464), float2(-0.38277543, 0.27676845), float2(0.97484398, 0.75648379),
	float2(0.44323325, -0.97511554), float2(0.53742981, -0.47373420), float2(-0.26496911, -0.41893023),
	float2(0.79197514, 0.19090188), float2(-0.24188840, 0.99706507), float2(-0.81409955, 0.91437590),
	float2(0.19984126, 0.78641367), float2(0.14383161, -0.14100790)};

static const float SMAP_WIDTH = 3840.0f ;
static const float SMAP_HEIGHT = 2160.0f;
static const float SMAP_DX = 1.0f / SMAP_WIDTH;
static const float SMAP_DY = 1.0f / SMAP_HEIGHT;

float calcShadowFactor(Texture2D shadowMap, float4 shadowPosH, float3 worldPos) {
	//if (shadowPosH.x < SMAP_DX*3.f || shadowPosH.x >= 1.0f - SMAP_DX*3.f) { 
	//	//not a U coordinate
	//	return 1.0f;
	//}
	//if (shadowPosH.y <= 0.0f || shadowPosH.y >= 1.0f) {
	//	// not a U coordinate
	//	return 1.0f;
	//}
	// Divide to get coordinates in texture projection
	//shadowPosH.xyz /= shadowPosH.w; //Not needed for orthographic

	// Depth in NDC
	float depth = shadowPosH.z - 0.001f;

	// Texel size
	const float dx = SMAP_DX;
	//float percentLit = 0.0f;
	float percentLit = 0.0f;
	const float2 offsets[9] = { 
		float2(-SMAP_DX, -SMAP_DY),	float2(0.0f, -SMAP_DY),	float2(SMAP_DX, -SMAP_DY),
		float2(-SMAP_DX, 0.0f),		float2(0.0f, 0.0f),		float2(SMAP_DX, 0.0f), 
		float2(-SMAP_DX, +SMAP_DY),	float2(0.0f, +SMAP_DY), float2(SMAP_DX, +SMAP_DY) };

	const float blur[9] = {
		0.077847f,
		0.123317f,
		0.077847f,
		0.123317f,
		0.195346f,
		0.123317f,
		0.077847f,
		0.123317f,
		0.077847f
	};
	// 3x3 box filter pattern. Each sample does a 4-tap PCF.
	for (int i = 0; i < 9; i++) {
		percentLit +=
			((shadowMap.Sample(samplerAni, shadowPosH.xy + offsets[i]) < depth).r) * blur[i];
	}
	//percentLit /= 9.0f;

	/*for (int i = 0; i < 4; i++) {
		int index = int(16.0 * random(floor(worldPos.xyz * 1000.0), i)) % 16;
		percentLit +=
			shadowMap.Sample(samplerAni, shadowPosH.xy + clamp(poissonDisk[index] / 700.0, float2(0.0f, 0.0f), float2(1.0f, 1.0f))).r < depth;
	}
	percentLit /= 4.0f;*/

	return (1 - percentLit);
}

float2 clampUV(uint2 uv, uint2 size) {
	return clamp(uv, uint2(0, 0), uint2(size.x - 1, size.y - 1));
}

float4 texSample(Texture2D texMap, uint2 texSize, float2 uv, float depthFromCamera) {
	float2 mappedUV = uv * (float2)texSize;
	uint2 floorUV = (uint2)mappedUV;
	float2 restUV = frac(mappedUV);

	float depth = depthFromCamera - 0.001f;

	float4 occluded = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 illuminated = float4(1.0, 1.0, 1.0, 1.0);
	uv = (float2)floorUV / texSize;
	float4 pixThis = texMap.Sample(samplerAni, uv).r < depth 
						  ? occluded 
						  : illuminated;
	float4 pixLeft = texMap.Sample(samplerAni, uv + float2(SMAP_DX, 0.0f)).r < depth 
						  ? occluded 
						  : illuminated;
	float4 pixRight = texMap.Sample(samplerAni, uv + float2(-SMAP_DX, 0.0f)).r < depth
						  ? occluded
						  : illuminated;
	float4 pixUp = texMap.Sample(samplerAni, uv + float2(0.0f, SMAP_DY)).r < depth
					   ? occluded
					   : illuminated;
	float4 pixDown = texMap.Sample(samplerAni, uv + float2(0.0f, -SMAP_DY)).r < depth
						 ? occluded
						 : illuminated;

	float4 pixRightUp =
		texMap.Sample(samplerAni, uv + float2(SMAP_DX, SMAP_DY)).r < depth ? occluded : illuminated;

	/*float4 horizontal = restUV.x > 0.5 ? (lerp(pixThis, pixLeft, (restUV.x -0.5f)*2.f))
									   : (lerp(pixRight, pixThis, restUV.x * 2.f));
	float4 vertical = restUV.y > 0.5 ? (lerp(pixThis, pixUp, (restUV.y - 0.5f)*2.f))
									 : (lerp(pixDown, pixThis, restUV.y*2.f));*/
	float4 horizontal1 = lerp(pixThis, pixRight, restUV.x);
	float4 horizontal2 = lerp(pixUp, pixRightUp, restUV.x);
	float4 vertical = lerp(horizontal1, horizontal2, restUV.y);
	return vertical;

	//return lerp(horizontal, vertical, 0.5);
}

float4 main(PS_IN ip) : SV_TARGET {
	//Texture sampling
	float3 aboveFlat = texture_aboveFlat.Sample(samplerAni, (ip.TexCoord * 50.)).rgb;
	float3 beneathFlat = texture_beneathFlat.Sample(samplerAni, (ip.TexCoord * 50.)).rgb;
	float3 aboveTilt = texture_aboveTilt.Sample(samplerAni, (ip.TexCoord * 50.f)).rgb;
	float3 betweenTiltAndFlat =
		texture_betweenTiltAndFlat.Sample(samplerAni, (ip.TexCoord * 50.)).rgb;
	// float3 beneathTilt = texture_beneathTilt.Sample(samplerAni, (ip.TexCoord * 50.) % 1.).rgb;

	// Colour interpolation values
	float height = specialLerp(ip.PosW.y, 0.1f, 1.f);
	float dotN = dot(float3(0, 1, 0), ip.Normal);
	float tilt = specialLerp(dotN, 0.60f, 0.70f);
	
	//Sample and shade from shadowmap
	//float shade = calcShadowFactor(texture_shadowMap, ip.ShadowPosH, ip.PosW);
	//float depth = ip.shadowPosH.z - 0.001f;
	
	float shade = texSample(texture_shadowMap, float2(3840.f, 2160.f), ip.ShadowPosH.xy, ip.ShadowPosH.z).r;
	
	//float3 lightToPos = ip.PosW - ;

	float3 flatColor = lerp(beneathFlat, aboveFlat, height);
	float3 baseTiltColor =
		lerp(aboveTilt, betweenTiltAndFlat, float3(1,1,1) * specialLerp(dotN, 0.45f, 0.55f));
	float3 baseColor =
		lerp(baseTiltColor, flatColor, float3(1, 1, 1) * specialLerp(dotN, 0.65f, 0.75f));

	return float4(lighting(ip.PosW, ip.Normal, baseColor, shade), 1.0);
	return float4(abs(ip.Normal), 1.0);
}