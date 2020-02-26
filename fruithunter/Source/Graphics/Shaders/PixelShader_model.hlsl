
struct PS_IN {
	float3 PosW : POSITION0;
	// float3 PosV : POSITION1;
	float4 PosH : SV_POSITION;
	float2 TexCoord : TEXCOORD;
	float3 Normal : NORMAL;
};

cbuffer materialbuffer : register(b2) {
	float4 ambient3;
	float4 diffuse3_strength;	// xyz diffuse, w strength for some strange reason
	float4 specular3_shininess; // xyz specular, w shininess
	float4 mapUsages;
}

Texture2D textures[3] : register(t0); // AmbientMap, DiffuseMap, SpecularMap
SamplerState samplerAni {
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Linear;
	AddressV = Linear;
};

float4 main(PS_IN ip) : SV_TARGET {
	// PS_OUT op = (PS_OUT)0;
	// op.normal = float4(normalize(ip.Normal), 1);
	// op.color = float4(color.rgb, 1);
	// op.position = float4(ip.PosW, 1);
	// op.specular = float4(0.5, 0.5, 0.5, 50);
	// op.viewPos = float4(ip.PosV, 1);
	// op.bloom = float4(0, 0, 0, 1);

	// light
	float3 lightPos = float3(-5, 2, -3);
	float3 toLight = normalize(lightPos - ip.PosW);
	toLight = float3(1, 1, 1);

	// ambient base
	float3 ambientBase =
		mapUsages.x ? (textures[0].Sample(samplerAni, ip.TexCoord)).rgb : (ambient3.rgb);

	// base color
	float3 pixelBaseColor =
		mapUsages.y ? (textures[1].Sample(samplerAni, ip.TexCoord)).rgb : (diffuse3_strength.rgb);

	// diffuse
	float diffuseTint = max(dot(toLight, ip.Normal), 0.0);

	// specular
	float3 specular = float3(0.0f, 0.0f, 0.0f);
	float reflectTint = 0.0f;
	if (diffuseTint > 0.0) {
		specular = mapUsages.z ? (textures[2].Sample(samplerAni, ip.TexCoord)).rgb
							   : (specular3_shininess.rgb);
		reflectTint =
			pow(max(dot(normalize(reflect(-toLight, ip.Normal)), normalize(-ip.PosW)), 0.0),
				specular3_shininess.w * 50);
	}

	// final color
	float3 col = pixelBaseColor * (0.2 + diffuseTint) + specular * reflectTint;
	return float4(col, 1.0);
	// return float4(ip.Normal,1.0);
}

static const float SMAP_SIZE = 2048.0f;
static const float SMAP_DX = 1.0f / SMAP_SIZE;

float calcShadowFactor(SamplerComparisonState samShadow, Texture2D shadowMap, float4 shadowPosH) {
	// Divide to get coordinates in texture projection
	shadowPosH.xyz /= shadowPosH.w;

	//Depth in NDC
	float depth = shadowPosH.z;

	// Texel size
	const float dx = SMAP_DX;
	float percentLit = 0.0f;
	const float2 offsets[9] = { 
		float2(-dx, -dx),	float2(0.0f, -dx),	float2(dx, -dx),
		float2(-dx, 0.0f),	float2(0.0f, 0.0f),	float2(dx, 0.0f), 
		float2(-dx, +dx),	float2(0.0f, +dx),	float2(dx, +dx) };
	//3x3 box filter pattern. Each sample does a 4-tap PCF.
	for (int i = 0; i < 9; i++) {
		percentLit += shadowMap.SampleCmpLevelZero(samShadow, shadowPosH.xy + offsets[i], depth).r;
	}
	return percentLit /= 9.0f;
}