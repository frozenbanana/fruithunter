
struct VS_IN {
	float3 Pos : Position;
	float2 TexCoord : TexCoordinate;
	float3 Normal : Normal;
};

struct VS_OUT {
	float3 PosW : POSITION0;
	float4 PosH : SV_POSITION;
	float2 TexCoord : TEXCOORD;
	float3 Normal : NORMAL;
	float4 ShadowPosH : POSITION2;
	//float3 PosV : POSITION1;
};

cbuffer cb_world : register(b0) { matrix mWorld, mInvTraWorld; };
cbuffer cb_viewPerspective : register(b1) { matrix mViewPerspective; };
cbuffer cb_shadowVPT : register(b4) { matrix mShadowVPT; };
cbuffer cb_viewPerspective : register(b5) { matrix mView, mPerspective; };

float2 rotate(float2 p, float r) {
	float2 rotated;
	rotated.x = p.x * cos(r) + p.y * sin(r);
	rotated.y = -p.x * sin(r) + p.y * cos(r);
	return rotated;
}

VS_OUT main(VS_IN input) {
	VS_OUT output = (VS_OUT)0;

	output.PosW = mul(float4(input.Pos, 1), mWorld).xyz;
	output.PosH = mul(float4(output.PosW, 1), mViewPerspective);
	//output.PosV = mul(float4(output.PosW, 1), mView).xyz;
	//output.PosH = mul(float4(output.PosV, 1), mPerspective);
	output.TexCoord = input.TexCoord;
	output.Normal = normalize(mul(float4(input.Normal, 0), mInvTraWorld).xyz);
	output.ShadowPosH = mul(float4(input.Pos, 1.0f), mWorld);
	output.ShadowPosH = mul(output.ShadowPosH, mShadowVPT);
	return output;
}