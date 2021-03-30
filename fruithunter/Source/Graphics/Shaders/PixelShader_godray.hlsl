
struct PS_IN {
	float3 PosW : POSITION0;
	float4 PosH : SV_POSITION;
	float2 TexCoord : TEXCOORD;
	float3 Normal : NORMAL;
	float4 ShadowPosH : POSITION2;
};

cbuffer colorBuffer : register(b8) { float4 color; }
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

cbuffer cameraProperties : register(b9) { float4 camera_position; }

float4 main(PS_IN ip) : SV_TARGET { 
	return float4(color.rgb, 1);
}