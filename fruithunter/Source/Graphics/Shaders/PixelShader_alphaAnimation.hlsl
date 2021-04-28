struct VS_OUT {
	float4 posH : SV_POSITION;
	float2 uv : TexCoord;
};

cbuffer AnimationSetting : register(b0) {
	float cb_factor;
	int cb_useBackground;
}

Texture2D tex_source : register(t0);
Texture2D tex_sourceBack : register(t1);
Texture2D tex_animation : register(t2);

SamplerState objSamplerState : SAMPLER : register(s0);

float4 main(VS_OUT ip) : SV_TARGET {
	float4 col = tex_source.Sample(objSamplerState, ip.uv).rgba;
	float alpha = tex_animation.Sample(objSamplerState, ip.uv).r;
	if (alpha == 1)
		return col; // original color
	else if (alpha == 0)
		return float4(0, 0, 0, 0); // black
	else {
		float4 colBack = float4(0, 0, 0, 0);
		if (cb_useBackground)
			colBack = tex_sourceBack.Sample(objSamplerState, ip.uv).rgba;
		if (1 - alpha < cb_factor) {
			alpha = clamp((cb_factor - (1 - alpha)) / 0.01f, 0, 1);
			return lerp(colBack, col, alpha);
		}
		else
			return colBack; // black
	}
}