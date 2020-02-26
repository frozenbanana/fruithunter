
struct VS_IN {
	float3 Pos0 : Position0;
	float2 TexCoord0 : TexCoordinate0;
	float3 Normal0 : Normal0;
	float3 Pos1 : Position1;
	float2 TexCoord1 : TexCoordinate1;
	float3 Normal1 : Normal1;
};

struct VS_OUT {
	float3 PosW : POSITION0;
	// float3 PosV : POSITION1;
	float4 PosH : SV_POSITION;
	float2 TexCoord : TEXCOORD;
	float3 Normal : NORMAL;
	float4 ShadowPosH : POSITION2;
};

SamplerComparisonState samShadow {
	Filter = COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
	AddressU = BORDER;
	AddressV = BORDER;
	AddressW = BORDER;
	BorderColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
	ComparionFunc = LESS_EQUAL;
};

cbuffer cb_world : register(b0) { matrix mWorld, mInvTraWorld; };
cbuffer cb_viewPerspective : register(b1) { matrix mViewPerspective; };
cbuffer cb_animation : register(b3) { float4 mAnimation; }; // x component is lerp factor
cbuffer cb_shadowVPT : register(b4) { float4x4 mShadowVPT; };

VS_OUT main(VS_IN input) {
	VS_OUT output = (VS_OUT)0;
	float3 pos = lerp(input.Pos0, input.Pos1, mAnimation.x);
	// float3 pos = lerp(input.Pos0, input.Pos1, 0.9);
	float3 normal = lerp(input.Normal0, input.Normal1, mAnimation.x);
	output.PosW = mul(float4(pos, 1), mWorld).xyz;
	output.PosH = mul(float4(output.PosW, 1), mViewPerspective);
	output.TexCoord = input.TexCoord0;
	output.Normal = normalize(mul(float4(normal, 0), mInvTraWorld).xyz);
	output.ShadowPosH = mul(float4(pos, 1.0f), mShadowVPT);

	return output;
}