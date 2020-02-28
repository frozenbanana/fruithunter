
struct VS_OUT {
	float4 PosV : SV_POSITION;
	float4 Color : Color;
};

struct GSOutput {
	float4 Pos : SV_POSITION;
	float4 Color : Color;
};

cbuffer cb_viewPerspective : register(b5) { matrix mView, mPerspective; };

[maxvertexcount(4)] void main(point VS_OUT input[1], inout TriangleStream<GSOutput> output) {
	float size = 1.0f;
	input[0].PosV;
	float3 playerPosition = float3(0.0f, 0.0f, 0.0f);
	float3 forward = normalize(playerPosition - input[0].PosV.xyz);
	float3 right = cross(forward, float3(0.f, 1.0f, 0.0));
	float up = cross(forward, right);

	// Question: Will 4 appending elements give a triangleStip?
	// Question: Since We are using posV, do we only need to use mPerspective now?
	// Make quad looking at the viewer

	// Calculate the corners
	// float3 right = float3(mView[0][0], mView[1][0], mView[2][0]);
	// float3 up = float3(mView[0][1], mView[1][1], mView[2][1]);
	float3 a = input[0].PosV - (right + up) * size;
	float3 b = input[0].PosV - (right - up) * size;
	float3 d = input[0].PosV + (right - up) * size;
	float3 c = input[0].PosV + (right + up) * size;

	// Append point to make a quad
	GSOutput element1;
	element1.Pos = mul(float4(a, 1.0f), mPerspective); // Place point to screen
	element1.Color = input[0].Color;
	output.Append(element1);

	GSOutput element2;
	element2.Pos = mul(float4(b, 1.0f), mPerspective);
	element2.Color = input[0].Color;
	output.Append(element2);

	GSOutput element3;
	element3.Pos = mul(float4(c, 1.0f), mPerspective);
	element3.Color = input[0].Color;
	output.Append(element3);

	GSOutput element4;
	element4.Pos = mul(float4(a, 1.0f), mPerspective);
	element4.Color = input[0].Color;
	output.Append(element4);
}