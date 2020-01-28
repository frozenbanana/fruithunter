
struct PS_IN {
	float3 PosW : POSITION0;
	float3 PosV : POSITION1;
	float4 PosH : SV_POSITION;
	float2 TexCoord : TEXCOORD;
	float3 Normal : NORMAL;
};
struct PS_OUT {
	float4 normal : SV_Target0;
	float4 color : SV_Target1;
	float4 position : SV_Target2;
	float4 specular : SV_TARGET3;
	float4 viewPos : SV_TARGET4;
	float4 bloom : SV_TARGET5;
};

cbuffer colorBuffer : register(b3) { float4 color; }

float4 main(PS_IN ip) : SV_TARGET {
	//PS_OUT op = (PS_OUT)0;
	//op.normal = float4(normalize(ip.Normal), 1);
	//op.color = float4(color.rgb, 1);
	//op.position = float4(ip.PosW, 1);
	//op.specular = float4(0.5, 0.5, 0.5, 50);
	//op.viewPos = float4(ip.PosV, 1);
	//op.bloom = float4(0, 0, 0, 1);

	return float4(ip.Normal.xyz,1.0);
}