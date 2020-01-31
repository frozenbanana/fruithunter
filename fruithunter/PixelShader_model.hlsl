
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

float3 lighting(float3 pos, float3 normal, float3 color) {
	// LIGHTING
	// light
	float3 lightPos = float3(-5, 2, -3);
	float3 toLight = normalize(lightPos - pos);
	// diffuse
	float shadowTint = max(dot(toLight, normal), 0.0);
	// specular
	// float reflectTint =
	//	pow(max(dot(normalize(reflect(-toLight, normal)), normalize(u_vp - pos)), 0.0), 20.0);
	// return color * (0.2 + shadowTint + reflectTint);
	return color * (0.2 + shadowTint);
}

float4 main(PS_IN ip) : SV_TARGET {
	// PS_OUT op = (PS_OUT)0;
	// op.normal = float4(normalize(ip.Normal), 1);
	// op.color = float4(color.rgb, 1);
	// op.position = float4(ip.PosW, 1);
	// op.specular = float4(0.5, 0.5, 0.5, 50);
	// op.viewPos = float4(ip.PosV, 1);
	// op.bloom = float4(0, 0, 0, 1);

	return float4(lighting(ip.PosW, ip.Normal.xyz, diffuse3_strength.xyz), 1.0);
	//return float4(ip.Normal,1.0);
}