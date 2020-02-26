
struct VS_IN {
	float3 Pos : Position;
	float2 TexCoord : TexCoordinate;
	float3 Normal : Normal;
};

struct VS_OUT {
	float3 PosW : POSITION0;
	// float3 PosV : POSITION1;
	float4 PosH : SV_POSITION;
	float2 TexCoord : TEXCOORD;
	float3 Normal : NORMAL;
	float4 ShadowPosH : POSITION2;
};

cbuffer cb_world : register(b0) { matrix mWorld, mInvTraWorld; };
cbuffer cb_viewPerspective : register(b1) { matrix mViewPerspective; };
cbuffer cb_shadowVPT : register(b4) { matrix mShadowVPT; };

VS_OUT main(VS_IN input) {
	VS_OUT output = (VS_OUT)0;

	output.PosW = mul(float4(input.Pos, 1), mWorld).xyz;
	output.PosH = mul(float4(output.PosW, 1), mViewPerspective);
	output.TexCoord = input.TexCoord;
	output.Normal = normalize(mul(float4(input.Normal, 0), mInvTraWorld).xyz);
	//output.ShadowPosH = mul(float4(input.Pos, 1.0f), mWorld);
	//output.ShadowPosH = mul(output.ShadowPosH, mShadowVPT);
	output.ShadowPosH = mul(float4(input.Pos, 1), mShadowVPT);
	//output.ShadowPosH = mul(output.ShadowPosH, mShadowVPT); 
	// mul(float4(input.Pos, 1), mShadowVPT);
	return output;
}