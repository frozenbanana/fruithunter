struct VS_OUT {
	float4 posH : SV_POSITION;
};

cbuffer screenSizeBuffer { float4 cb_screenSize; };

Texture2D<float> depthMap : register(s0);

float linearDepth(float depthSample) {
	const float zNear = 0.025f;
	const float zFar = 100.f;
	depthSample = 2.0 * depthSample - 1.0;
	float zLinear = 2.0 * zNear * zFar / (zFar + zNear - depthSample * (zFar - zNear));
	return zLinear;
}

float4 main(VS_OUT ip) : SV_TARGET {
	float alpha = 0.f;
	float3 color = float3(0, 0, 0);

	uint2 screenSize = uint2(cb_screenSize.x, cb_screenSize.y);

	float2 uv = float2(ip.posH.x / screenSize.x, ip.posH.y / screenSize.y);

	float pixThis = linearDepth(depthMap[ip.posH.xy]);
	//int size = clamp((1 - clamp((pixThis - 0.025) / 100.f, 0, 1)) * 5.f, 3, 5);
	int size = 5;
	for (int xx = -size / 2; xx < size / 2; xx++) {
		for (int yy = -size / 2; yy < size / 2; yy++) {
			float pixOther = linearDepth(depthMap[ip.posH.xy + int2(xx,yy)]);
			if ((pixThis - pixOther) > 0.15f*pixThis) {
				alpha = 1.f;
			}
		}
	}
	
	return float4(color, alpha); 

}