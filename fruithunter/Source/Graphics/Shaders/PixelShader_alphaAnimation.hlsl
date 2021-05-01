struct VS_OUT {
	float4 posH : SV_POSITION;
	float2 uv : TexCoord;
};

cbuffer AnimationSetting : register(b0) {
	float cb_factor;
	float cb_fadeLength;
	int cb_useBackground;
}

Texture2D tex_source : register(t0);
Texture2D tex_sourceBack : register(t1);
Texture2D tex_animation : register(t2);

SamplerState objSamplerState : SAMPLER : register(s0);

float4 main(VS_OUT ip) : SV_TARGET {
	float factor = 1 - cb_factor;
	float4 col = tex_source.Sample(objSamplerState, ip.uv).rgba;
	float alpha = tex_animation.Sample(objSamplerState, ip.uv).r;
	float4 colBack = float4(0, 0, 0, 0);
	if (cb_useBackground)
		colBack = tex_sourceBack.Sample(objSamplerState, ip.uv).rgba;
	if (alpha >= factor) {
		// visible
		return col;
	}
	else {
		if (cb_fadeLength == 0)
			return colBack;
		else {
			alpha = clamp(abs(factor - alpha) / cb_fadeLength, 0, 1);
			// invisible, if not faded
			return lerp(col, colBack, alpha); // black
		}
	}
}