
struct PS_IN {
	float3 PosW : POSITION0;
	float3 PosV : POSITION1;
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

float3 lighting(float3 pos, float3 normal, float3 color, float shade) {
	// light utility
	/*float3 lightPos = float3(-0.f, 110.f, 100.f);
	float3 toLight = normalize(lightPos - pos);*/
	float3 toLight = normalize(float3(-100.f, 110.f, 0));

	// diffuse
	float shadowTint = max(dot(toLight, normal), 0.0);

	// specular
	// float reflectTint =
	//	pow(max(dot(normalize(reflect(-toLight, normal)), normalize(-pos)), 0.0), 20.0);
	// return color * (0.2 + shadowTint + reflectTint);
	return color * (ambient.xyz + shadowTint * shade * diffuse.xyz);
}

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

float4 texSampleGrease(Texture2D texMap, uint2 texSize, float2 uv, float depthFromCamera, float3 posW) {
	float2 mappedUV = uv * (float2)texSize;
	uint2 floorUV = (uint2)mappedUV;
	float2 restUV = frac(mappedUV);
	float2 mapDelta = float2(1.0f / texSize.x, 1.0f / texSize.y);

	float depth_linear = linearDepth(depthFromCamera - 0.001f);

	uv = (float2)floorUV / texSize;
	float2 external = (1.0f*float2(random(posW, 1), random(posW, 2)) + restUV.xy) * mapDelta;
	float sampledDepth_linear = linearDepth(texMap.Sample(samplerAni, uv + external).r);

	return sampledDepth_linear < depth_linear ? 0.0f : 1.f;
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
	float shade = texSampleGrease(texture_shadowMap, float2(3840.f, 2160.f), ip.ShadowPosH.xy, ip.ShadowPosH.z, ip.PosW.xyz).r;


	float3 flatColor = lerp(beneathFlat, aboveFlat, height);
	float3 baseTiltColor =
		lerp(aboveTilt, betweenTiltAndFlat, float3(1,1,1) * specialLerp(dotN, 0.45f, 0.55f));
	float3 baseColor =
		lerp(baseTiltColor, flatColor, float3(1, 1, 1) * specialLerp(dotN, 0.65f, 0.75f));

	return float4(lighting(ip.PosW, ip.Normal, baseColor, shade), 1.0);
	return float4(abs(ip.Normal), 1.0);}