struct VS_IN {
	float3 Pos : Position;
	float2 TexCoord : TexCoordinate;
	float3 Normal : Normal;
};

struct VS_OUT {
	float4 PosH : SV_POSITION;
	float2 TexCoord : TEXCOORD;
};

cbuffer cb_viewPerspective : register(b1) { matrix mView, mPerspective, mViewPerspective; };

VS_OUT main(VS_IN input) {
	VS_OUT output = (VS_OUT)0;

	output.PosH = mul(float4(input.Pos, 0), mViewPerspective);
	output.PosH.z = output.PosH.w*0.999999; //If Z is 1, it disappears. Thanks Linus.

	output.TexCoord = input.TexCoord;

	return output;
}