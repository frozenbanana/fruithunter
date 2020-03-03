struct VS_IN {
	float3 pos : Position;
};

struct VS_OUT {
	float4 posH : SV_POSITION;
};

VS_OUT main(VS_IN pos) {
	VS_OUT output;
	output.posH = float4(pos, 1.f);
	return output;
}