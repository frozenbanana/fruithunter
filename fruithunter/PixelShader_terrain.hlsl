
struct PS_IN {
	float3 PosW : POSITION0;
	// float3 PosV : POSITION1;
	float4 PosH : SV_POSITION;
	float2 TexCoord : TEXCOORD;
	float3 Normal : NORMAL;
};

float3 lighting(float3 pos, float3 normal, float3 color) {
	// LIGHTING
	// light
	float3 lightPos = float3(-5, 2, -3);
	float3 toLight = normalize(lightPos - pos);
	// diffuse
	float shadowTint = max(dot(toLight, normal), 0.0);
	// specular
	//float reflectTint =
	//	pow(max(dot(normalize(reflect(-toLight, normal)), normalize(-pos)), 0.0), 20.0);
	//return color * (0.2 + shadowTint + reflectTint);
	return color * (0.2 + shadowTint);
}

float4 main(PS_IN ip) : SV_TARGET {
	float3 green = float3(0,1,0.3);
	float3 rock = float3(1,1,1)*0.7;
	float c = dot(float3(0,1,0),ip.Normal);
	float3 baseColor = lerp(rock,green,float3(1,1,1)*c);
	return float4(lighting(ip.PosW, ip.Normal, baseColor), 1.0);
	//return float4(ip.Normal,1.0);
}