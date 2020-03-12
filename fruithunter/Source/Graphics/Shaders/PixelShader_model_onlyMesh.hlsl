
struct PS_IN {
	float3 PosW : POSITION0;
	// float3 PosV : POSITION1;
	float4 PosH : SV_POSITION;
	float2 TexCoord : TEXCOORD;
	float3 Normal : NORMAL;
	float4 ShadowPosH : POSITION2;
};

cbuffer colorBuffer : register(b2) { float4 color; }
Texture2D texture_shadowMap : register(t4);
cbuffer lightInfo : register(b5) {
	float4 ambient;
	float4 diffuse;
	float4 specular;
};

cbuffer lightInfo : register(b6) {
	float2 cb_shadowMapRes;
	float2 cb_nearFarPlane;
	float4 cb_toLight;
};

SamplerState samplerAni {
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Linear;
	AddressV = Linear;
};

float specialLerp(float v, float min, float max) {
	return (v > max ? 1.f : (v < min ? 0.f : (v - min) / (max - min)));
}

float random(float3 seed, int i) {
	float4 seed4 = float4(seed, i);
	float dot_product = dot(seed4, float4(12.9898, 78.233, 45.164, 94.673));
	return frac(sin(dot_product) * 43758.5453);
}

float linearDepth(float depthSample) {
	const float zNear = 0.025f;
	const float zFar = 100.f;
	depthSample = 2.0 * depthSample - 1.0;
	float zLinear = 2.0 * zNear * zFar / (zFar + zNear - depthSample * (zFar - zNear));
	return zLinear;
}

float4 texSampleGrease(
	Texture2D texMap, uint2 texSize, float2 uv, float depthFromCamera, float3 posW) {
	if (uv.x >= 0 && uv.x <= 1 && uv.y >= 0 && uv.y <= 1) {
		float2 mappedUV = uv * (float2)texSize;
		int2 floorUV = (int2)mappedUV;
		float2 restUV = frac(mappedUV);
		float2 mapDelta = float2(1.0f / texSize.x, 1.0f / texSize.y);

		float depth_linear = linearDepth(depthFromCamera - 0.001f);

		uv = (float2)floorUV / texSize;
		float2 external = (1.0f * float2(random(posW, 1), random(posW, 2)) + restUV.xy) * mapDelta;
		float sampledDepth_linear = linearDepth(texMap.Sample(samplerAni, uv + external).r);

		return sampledDepth_linear < depth_linear ? 0.0f : 1.f;
	}
	return 1.f;
}

float3 lighting(float3 pos, float3 normal, float3 color, float shade) {
	// light utility
	/*float3 lightPos = float3(-0.f, 110.f, 100.f);
	float3 toLight = normalize(lightPos - pos);*/
	float3 toLight = normalize(cb_toLight.xyz);

	// diffuse
	float shadowTint = max(dot(toLight, normal), 0.0);

	// specular
	// float reflectTint =
	//	pow(max(dot(normalize(reflect(-toLight, normal)), normalize(-pos)), 0.0), 20.0);
	// return color * (0.2 + shadowTint + reflectTint);
	return color * (ambient.xyz + shadowTint * shade * diffuse.xyz);
}

float4 main(PS_IN ip) : SV_TARGET {
	float shade = texSampleGrease(
		texture_shadowMap, cb_shadowMapRes, ip.ShadowPosH.xy, ip.ShadowPosH.z, ip.PosW.xyz)
					  .r;
	return float4(lighting(ip.PosW, ip.Normal.xyz, color.xyz, shade), 1.0);
}