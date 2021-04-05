struct VS_IN {
	float3 Pos : Position;
	float4 Color : Color;
	float Size : Size;
	float IsActive : IsActive;
};

struct VS_OUT {
	float4 PosV : SV_POSITION;
	float4 Color : Color;
	float Size : Size;
	float IsActive : IsActive;
};


cbuffer cb_viewPerspective : register(b1) { matrix mView, mPerspective, mViewPerspective; };

VS_OUT main(VS_IN input) {
	VS_OUT output;

	output.PosV = mul(float4(input.Pos, 1.f), mView);
	output.Size = input.Size;
	output.Color = input.Color;
	output.IsActive = input.IsActive;
	return output;
}