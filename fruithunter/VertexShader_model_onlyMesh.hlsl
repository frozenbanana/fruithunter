
struct VS_IN {
	float3 Pos : Position;
	float2 TexCoord : TexCoordinate;
	float3 Normal : Normal;
};

struct VS_OUT {
	float3 PosW : POSITION0;
	float3 PosV : POSITION1;
	float4 PosH : SV_POSITION;
	float2 TexCoord : TEXCOORD;
	float3 Normal : NORMAL;
};
//cbuffer cbMatrix : register(b0) {
//	matrix mWorld, mView, mPerspective;
//	matrix mInvTraWorld;
//};
cbuffer cb_world : register(b0) {
	matrix mWorld, mInvTraWorld;
};
cbuffer cb_viewPerspective : register(b1) {
	matrix mView, mPerspective;
};

VS_OUT main( VS_IN input )
{
	VS_OUT output = (VS_OUT)0;

	//output.PosW = mul(float4(input.Pos, 1), mWorld).xyz;
	//output.PosV = mul(float4(output.PosW, 1), mView).xyz;
	//output.PosH = mul(float4(output.PosV, 1), mPerspective);
	//output.TexCoord = input.TexCoord;
	//output.Normal = mul(float4(input.Normal, 0), mInvTraWorld).xyz;

	 output.PosW = mul(float4(input.Pos, 1), mWorld).xyz;
	 output.PosV = mul(float4(output.PosW, 1), mView).xyz;
	 output.PosH = mul(float4(output.PosV, 1), mPerspective);
	 output.TexCoord = input.TexCoord;
	 output.Normal = mul(float4(input.Normal, 0), mul(mView,mWorld)).xyz;

	return output;
}