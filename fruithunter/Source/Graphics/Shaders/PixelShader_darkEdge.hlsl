struct VS_OUT {
	float4 posH : SV_POSITION;
	float2 uv : TexCoord;
};

cbuffer screenSizeConstantBuffer : register(b9) { int4 cb_screenSize; };

Texture2D<float> depthMap : register(s0);

float linearDepth(float depthSample) {
	const float zNear = 0.025f;
	const float zFar = 100.f;
	depthSample = 2.0 * depthSample - 1.0;
	float zLinear = 2.0 * zNear * zFar / (zFar + zNear - depthSample * (zFar - zNear));
	return zLinear;
}

float4 main(VS_OUT ip) : SV_TARGET {
	float blend = 1.f;
	float3 edgeColor = float3(0, 0, 0);

	float pixThis = linearDepth(depthMap[ip.posH.xy]);
	//float pixThisNorm = clamp((pixThis - 0.025) / (150-0.025), 0, 1);
	//float maxD = 5, minD = 0;
	//float size = max((1 - pixThisNorm) * maxD, minD);
	//float rest = frac(size);
	//int isize = ceil(size);

	float table[] = {
		0, 1, 2, 1, 0,
		1, 2, 3, 2, 1,
		2, 3, 0, 3, 2,
		1, 2, 3, 2, 1,
		0, 1, 2, 1, 0,
	};

	float sum = 0;
	float total = 0;
	int isize = 5;
	for (int xx = 0; xx < isize; xx++) {
		for (int yy = 0; yy < isize; yy++) {
			total += table[yy * isize + xx];
			int2 uv = ip.posH.xy + int2(xx - (int)(isize / 2), yy - (int)(isize / 2));
			if (uv.x >= 0 && uv.x < cb_screenSize.x && uv.y >= 0 && uv.y < cb_screenSize.y) {
				float pixOther = linearDepth(depthMap[uv]);
				if ((pixThis - pixOther) > 0.15f * pixThis) {
					sum += table[yy * isize + xx];
				}
			}
		}
	}
	blend = clamp(5*sum / total, 0, 1);

	return float4(edgeColor, blend);
}