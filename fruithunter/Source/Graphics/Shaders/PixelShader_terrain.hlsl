
struct PS_IN {
	float3 PosW : POSITION0;
	// float3 PosV : POSITION1;
	float4 PosH : SV_POSITION;
	float2 TexCoord : TEXCOORD;
	float3 Normal : NORMAL;
	//float Height : LOCAL_Y;
};

SamplerState samplerAni {
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = ADDRESS_CLAMP;
	AddressV = ADDRESS_CLAMP;
};
Texture2D texture_aboveFlat : register(t0);
Texture2D texture_beneathFlat : register(t1);
Texture2D texture_aboveTilt : register(t2);
Texture2D texture_betweenTiltAndFlat : register(t3);

float3 lighting(float3 pos, float3 normal, float3 color) {
	// LIGHTING
	// light
	float3 lightPos = float3(-5, 2, -3);
	float3 toLight = normalize(lightPos - pos);
	toLight = float3(1, 1, 1);
	// diffuse
	float shadowTint = max(dot(toLight, normal), 0.0);
	// specular
	//float reflectTint =
	//	pow(max(dot(normalize(reflect(-toLight, normal)), normalize(-pos)), 0.0), 20.0);
	//return color * (0.2 + shadowTint + reflectTint);
	return color * (0.2 + shadowTint);
}

float specialLerp(float v, float min, float max) {
	return (v > max ? 1.f : (v < min ? 0.f : (v - min) / (max - min)));
}

float4 main(PS_IN ip) : SV_TARGET {
	float3 aboveFlat = texture_aboveFlat.Sample(samplerAni, (ip.TexCoord * 50.) % 1.).rgb;
	float3 beneathFlat = texture_beneathFlat.Sample(samplerAni, (ip.TexCoord * 50.) % 1.).rgb;
	float3 aboveTilt = texture_aboveTilt.Sample(samplerAni, (ip.TexCoord * 50.f) % 1.).rgb;
	float3 betweenTiltAndFlat =
		texture_betweenTiltAndFlat.Sample(samplerAni, (ip.TexCoord * 50.) % 1.).rgb;
	//float3 beneathTilt = texture_beneathTilt.Sample(samplerAni, (ip.TexCoord * 50.) % 1.).rgb;

	float height = specialLerp(ip.PosW.y, 0.1f, 1.f);
	float dotN = dot(float3(0, 1, 0), ip.Normal);
	float tilt = specialLerp(dotN, 0.60f, 0.70f);

	float3 flatColor = lerp(beneathFlat, aboveFlat, height);
	float3 baseTiltColor =
		lerp(aboveTilt, betweenTiltAndFlat, float3(1, 1, 1) * specialLerp(dotN, 0.45f, 0.55f));
	float3 baseColor =
		lerp(baseTiltColor, flatColor, float3(1, 1, 1) * specialLerp(dotN, 0.65f, 0.75f));

	return float4(lighting(ip.PosW, ip.Normal, baseColor), 1.0);
	return float4(abs(ip.Normal),1.0);
}