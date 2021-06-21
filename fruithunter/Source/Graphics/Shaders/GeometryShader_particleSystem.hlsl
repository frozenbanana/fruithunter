
struct VS_OUT {
	float4 PosV : SV_POSITION;
	float Rotation : Rotation;
	float4 Color : Color;
	float Size : Size;
	float IsActive : IsActive;
};

struct GSOutput {
	float4 PosH : SV_POSITION;
	float4 Color : Color;
	float2 UV : UV;
};

cbuffer cb_viewPerspective : register(b1) { matrix mView, mPerspective, mViewPerspective; };

struct Corner {
	float4 pos;
	float2 uv;
};

float4x4 axis_matrix(float3 right, float3 up, float3 forward) {
	float3 xaxis = right;
	float3 yaxis = up;
	float3 zaxis = forward;
	return float4x4(xaxis.x, yaxis.x, zaxis.x, 0, xaxis.y, yaxis.y, zaxis.y, 0, xaxis.z, yaxis.z,
		zaxis.z, 0, 0, 0, 0, 1);
}

float4x4 look_at_matrix(float3 forward, float3 up) {
	float3 xaxis = normalize(cross(forward, up));
	float3 yaxis = up;
	float3 zaxis = forward;
	return axis_matrix(xaxis, yaxis, zaxis);
}

[maxvertexcount(4)] void main(point VS_OUT input[1], inout TriangleStream<GSOutput> output) {
	if (abs(1.f - input[0].IsActive) < 0.1f) {
		float size = input[0].Size;
		float rot = input[0].Rotation;
		float3 posV = input[0].PosV.xyz;
		float3 playerPosition = float3(0.0f, 0.0f, 0.0f);
		float3 forward = normalize(playerPosition - posV);
		float3 right = cross(forward, float3(0.f, 1.0f, 0.f));
		float3 up = cross(right, forward);

		float4x4 mat = axis_matrix(right, up, forward);
		right = mul(float4(cos(rot), sin(rot), 0, 0), mat);
		float quarter = 3.1415f / 2.f;
		up = mul(float4(cos(rot + quarter), sin(rot + quarter), 0, 0), mat);

		// Calculate the corners
		float halfWidth = 0.5f * size;
		float halfHeight = 0.5f * size;

		Corner corners[4];
		corners[0].pos = float4(posV + halfWidth * right - halfHeight * up, 1.0f);
		corners[0].uv = float2(1.0f, 0.0f);
		corners[1].pos = float4(posV - halfWidth * right - halfHeight * up, 1.0f);
		corners[1].uv = float2(0.0f, 0.0f);
		corners[2].pos = float4(posV + halfWidth * right + halfHeight * up, 1.0f);
		corners[2].uv = float2(1.0f, 1.0f);
		corners[3].pos = float4(posV - halfWidth * right + halfHeight * up, 1.0f);
		corners[3].uv = float2(0.0f, 1.0f);

		// Append points to make a quad
		GSOutput element;
		for (int i = 0; i < 4; i++) {
			element.PosH = mul(corners[i].pos, mPerspective);
			element.Color = input[0].Color;
			element.UV = corners[i].uv;
			output.Append(element);
		}
	}
}