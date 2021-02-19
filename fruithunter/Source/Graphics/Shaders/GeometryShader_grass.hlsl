struct VS_OUT {
	float3 PosV : POSITION0;
	float rotation : RotationY;
	float height : Height;
};

struct GSOutput {
	float4 PosW : POSITION0;
	float4 PosH : SV_POSITION;
	float4 Color : Color;
	float2 uv : UV;
	float4 ShadowPosH : POSITION2;
};

cbuffer strawSettings : register(b5) { 
	float cb_baseWidth;
	float cb_minHeight;
	float cb_maxHeight;
	float filler;
	float4 cb_color_top; 
	float4 cb_color_bottom; 
}

cbuffer cb_world : register(b0) { matrix mWorld, mInvTraWorld; };
cbuffer cb_viewPerspective : register(b1) { matrix mView, mPerspective, mViewPerspective; };
cbuffer cb_shadowVPT : register(b4) { matrix mShadowVPT; };

[maxvertexcount(3)] 
void main(point VS_OUT input[1], inout TriangleStream<GSOutput> output) {

	float3 posW = mul(float4(input[0].PosV, 1), mWorld);
	float rotation = input[0].rotation;
	float height = cb_minHeight + input[0].height * (cb_maxHeight - cb_minHeight);
	float width = cb_baseWidth;

	float3 up = float3(0, 1, 0);
	float3 dir1 = float3(cos(rotation), 0, sin(rotation));
	float3 dir2 = float3(cos(rotation + 3.1415f / 2.f), 0, sin(rotation + 3.1415f / 2.f));

	float3 positions[3];
	float2 uvs[3];
	positions[0] = posW + up * 0.f + dir1 * width;
	positions[1] = posW + up * 0.f - dir1 * width;
	positions[2] = posW + up * height + dir1 * 0.f;

	//positions[3] = posW + up * 0.f + dir2 * width;
	//positions[4] = posW + up * 0.f - dir2 * width;
	//positions[5] = posW + up * height + dir2 * 0.f;

	GSOutput e;
	for (int i = 0; i < 3; i++) {
		e.PosW = float4(positions[i], 1);
		e.PosH = mul(float4(positions[i], 1), mViewPerspective);
		e.ShadowPosH = mul(float4(positions[i], 1), mShadowVPT);
		e.Color = (i%3 == 2) ? cb_color_top : cb_color_bottom;
		e.uv = float2(0, 0);
		output.Append(e);
	}
}