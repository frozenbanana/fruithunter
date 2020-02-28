struct VS_IN {
	float3 Pos : Position;
	float4 Color : Color;
	float Size : Size;
};

struct VS_OUT {
	float4 PosV : SV_POSITION;
	float4 Color : Color;
	float Size : Size;
};


cbuffer cb_viewPerspective : register(b5) { matrix mView, mPerspective; };

VS_OUT main(VS_IN input) {
	VS_OUT output;

	output.PosV = mul(float4(input.Pos, 1.f), mView);
	output.Size = input.Size;
	output.Color = input.Color;
	return output;
}