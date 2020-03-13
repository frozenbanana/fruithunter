
struct PS_IN {
	float3 PosW : POSITION0;
	// float3 PosV : POSITION1;
	float4 PosH : SV_POSITION;
	float2 TexCoord : TEXCOORD;
	float3 Normal : NORMAL;
	float4 ShadowPosH : POSITION2;
};

Texture2D textMap : register(t0); // text texture
//cbuffer lightInfo : register(b5) {
//	float4 ambientColour;
//	float4 diffuseColour;
//	float4 specularColour;
//};
//
//cbuffer lightInfo : register(b6) {
//	float2 cb_shadowMapRes;
//	float2 cb_nearFarPlane;
//	float4 cb_toLight;
//};

SamplerState samplerAni;

float4 main(PS_IN ip) : SV_TARGET {

	//float3 toLight = normalize(cb_toLight.xyz);

	// base color
	float4 pixelBaseColor = textMap.Sample(samplerAni, ip.TexCoord);
	// diffuse
	//float diffuseTint = max(dot(toLight, ip.Normal), 0.0);

	// final color
	//float3 col = pixelBaseColor.rgb * (ambientColour.xyz + diffuseTint * shade * diffuseColour.xyz);
	return float4(pixelBaseColor.rgb, pixelBaseColor.a);
}