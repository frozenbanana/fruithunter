struct VS_IN {
	float3 Pos : Position;
	float Rotation : Rotation;
	float4 Color : Color;
	float Size : Size;
	int IsActive : IsActive;
};

struct VS_OUT {
	float4 PosV : SV_POSITION;
	float Rotation : Rotation;
	float4 Color : Color;
	float Size : Size;
	int IsActive : IsActive;
};


cbuffer cb_viewPerspective : register(b1) { matrix mView, mPerspective, mViewPerspective; };

VS_OUT main(VS_IN input) {
	VS_OUT output;

	output.PosV = mul(float4(input.Pos, 1.f), mView);
	output.Rotation = input.Rotation;
	output.Size = input.Size;
	output.Color = input.Color;
	output.IsActive = input.IsActive;
	return output;
}