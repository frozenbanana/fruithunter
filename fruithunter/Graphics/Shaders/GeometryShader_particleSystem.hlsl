
struct VS_OUT {
	float3 PosV : SV_POSITION;
	float4 Color : SV_COLOR;
};

struct GSOutput {
	float4 Pos : SV_POSITION;
	float4 Color : SV_COLOR;
};

cbuffer cb_viewPerspective : register(b5) { matrix mView, mPerspective; };

[maxvertexcount(1)] void main(point VS_OUT input, inout TriangleStream<GSOutput> output) {
	float size = 1.0f;
	float3 playerPosition = float3(0.0f, 0.0f, 0.0f);
	// Question: Will 4 appending elements give a triangleStip?
	// Question: Since We are using posV, do we only need to use mPerspective now?
	// Make quad looking at the viewer

	// Calculate the corners
	float3 right = float3(mView[0][0], mView[1][0], mView[2][0]);
	float3 up = float3(mView[0][1], mView[1][1], mView[2][1]);
	float3 a = input.PosV - (right + up) * size;
	float3 b = input.PosV - (right - up) * size;
	float3 d = input.PosV + (right - up) * size;
	float3 c = input.PosV + (right + up) * size;

	// Append point to make a quad
	GSOutput element1;
	element.Pos = mul(float4(a, 1.0f), mPerspective); // Place point to screen
	element.Color = input.Color;
	output.Append(element1);

	GSOutput element2;
	element.Pos = mul(float4(b, 1.0f), mPerspective);
	element.Color = input.Color;
	output.Append(element2);

	GSOutput element3;
	element.Pos = mul(float4(c, 1.0f), mPerspective);
	element.Color = input.Color;
	output.Append(element3);

	GSOutput element4;
	element.Pos = mul(float4(a, 1.0f), mPerspective);
	element.Color = input.Color;
	output.Append(element4);
}