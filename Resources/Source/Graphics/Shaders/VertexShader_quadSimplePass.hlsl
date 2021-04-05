struct VS_IN {
	float2 posH : Position;
	float2 uv : TexCoord;
};

struct VS_OUT {
	float4 posH : SV_POSITION;
	float2 uv : TexCoord;
};

VS_OUT main(VS_IN ip) {
	VS_OUT output;
	output.posH = float4(ip.posH.xy, 0.f, 1.f);
	output.uv = ip.uv;
	return output;
}