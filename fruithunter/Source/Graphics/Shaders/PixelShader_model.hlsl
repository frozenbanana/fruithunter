
struct PS_IN {
	float3 PosW : POSITION0;
	// float3 PosV : POSITION1;
	float4 PosH : SV_POSITION;
	float2 TexCoord : TEXCOORD;
	float3 Normal : NORMAL;
	float4 ShadowPosH : POSITION2;
};

cbuffer materialbuffer : register(b2) {
	float4 ambient3;
	float4 diffuse3_strength;	// xyz diffuse, w strength for some strange reason
	float4 specular3_shininess; // xyz specular, w shininess
	float4 mapUsages;
}

Texture2D textures[3] : register(t0); // AmbientMap, DiffuseMap, SpecularMap
Texture2D texture_shadowMap : register(t4);
cbuffer lightInfo : register(b5) {
	float4 ambientColour;
	float4 diffuseColour;
	float4 specularColour;
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

float texSampleGrease(
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

float4 main(PS_IN ip) : SV_TARGET {
	float3 toLight = normalize(cb_toLight.xyz);

	// base color
	float3 pixelBaseColor =
		mapUsages.y ? (textures[1].Sample(samplerAni, ip.TexCoord)).rgb : (diffuse3_strength.rgb);
	if (mapUsages.y && (textures[1].Sample(samplerAni, ip.TexCoord)).a == 0)
		return float4(0, 0, 0, 0);

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

	float shade = texSampleGrease(
		texture_shadowMap, cb_shadowMapRes, ip.ShadowPosH.xy, ip.ShadowPosH.z, ip.PosW.xyz)
					  .r;

	// final color
	float3 col = pixelBaseColor * ((ambientColour.xyz + diffuseTint * shade * diffuseColour.xyz) +
									  specular * reflectTint * specularColour.xyz);
	return float4(col, 1.0);
}