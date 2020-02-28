
struct VS_OUT {
	float4 PosV : SV_POSITION;
	float4 Color : Color;
	float Size : Size;
};

struct GSOutput {
	float4 PosH : SV_POSITION;
	float4 Color : Color;
	float2 UV : UV;
	float Size : Size;
};

cbuffer cb_viewPerspective : register(b5) { matrix mView, mPerspective; };

struct Corner {
	float4 pos;
	float2 uv;
};

[maxvertexcount(4)] void main(point VS_OUT input[1], inout TriangleStream<GSOutput> output) {
	float size = input[0].Size;
	float3 posV = input[0].PosV.xyz;
	float3 playerPosition = float3(0.0f, 0.0f, 0.0f);
	float3 forward = normalize(playerPosition - posV);
	float3 right = cross(forward, float3(0.f, 1.0f, 0.0));
	float3 up = cross(forward, right);

	// Calculate the corners
	float halfWidth = 0.5f * size;
	float halfHeight = 0.5f * size;

	Corner corners[4];
	corners[0].pos = float4(posV + halfWidth * right - halfHeight * up, 1.0f);
	corners[0].uv = float2(0.0f, 0.0f);
	corners[1].pos = float4(posV + halfWidth * right + halfHeight * up, 1.0f);
	corners[1].uv = float2(1.0f, 0.0f);
	corners[2].pos = float4(posV - halfWidth * right - halfHeight * up, 1.0f);
	corners[2].uv = float2(0.0f, 1.0f);
	corners[3].pos = float4(posV - halfWidth * right + halfHeight * up, 1.0f);
	corners[3].uv = float2(1.0f, 1.0f);

	// Append points to make a quad
	GSOutput element;
	for (int i = 0; i < 4; i++) {
		element.PosH = mul(corners[i].pos, mPerspective);
		element.Color = input[0].Color;
		element.UV = corners[i].uv;
		element.Size = input[0].Size;
		output.Append(element);
	}
}