cbuffer MatrixBuffer : register(b1) { matrix viewProjMatrix; };

struct Input {
	float2 position : POSITION;
	float3 color : COLOR;
	float2 uv : TEXCOORD;
};

struct Output {
	float4 position : SV_POSITION;
	float3 color : COLOR;
	float2 uv : TEXCOORD;
};

Output main(Input input) {
	Output output;

	float4 pos = float4(input.position.x, input.position.y, 0, 1);
	// output.position = pos;
	output.position = mul(viewProjMatrix, pos);

	output.color = input.color;
	output.uv = input.uv;
	return output;
}
