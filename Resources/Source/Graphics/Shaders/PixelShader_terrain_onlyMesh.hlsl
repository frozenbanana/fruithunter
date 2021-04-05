struct VS_OUT {
	float3 PosW : POSITION0;
	// float3 PosV : SV_POSITION1;
	float4 PosH : SV_POSITION;
	float2 TexCoord : TEXCOORD;
	float3 Normal : NORMAL;
};

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

float3 lighting(float3 pos, float3 normal, float3 color) {
	// light utility
	float3 toLight = normalize(cb_toLight.xyz);
	// diffuse
	float shadowTint = max(dot(toLight, normal), 0.0);

	return color * (ambient.xyz + shadowTint * diffuse.xyz);
}

float4 main(VS_OUT ip) : SV_TARGET {
	return float4(lighting(ip.PosW, ip.Normal, float3(1.f, 1.f, 1.f)), 1.0f);
}