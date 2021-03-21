struct VS_OUT {
	float4 posH : SV_POSITION;
};

cbuffer screenSizeBuffer : register(b9) { int4 cb_screenSize; };

cbuffer frustum : register(b8) {
	float4 cb_topLeft;
	float4 cb_topRight;
	float4 cb_bottomLeft;
	float4 cb_bottomRight;
}

Texture2D<float> depthMap : register(s0);

SamplerState samp;

float linearDepth(float depthSample) {
	const float zNear = 0.025f;
	const float zFar = 100.f;
	depthSample = 2.0 * depthSample - 1.0;
	float zLinear = 2.0 * zNear * zFar / (zFar + zNear - depthSample * (zFar - zNear));
	return zLinear;
}

float3 uvToViewPoint(float2 uv) {
	float depth = linearDepth(depthMap.Sample(samp, uv));
	float4 top = lerp(cb_topLeft, cb_topRight, uv.x);
	float4 bottom = lerp(cb_bottomLeft, cb_bottomRight, uv.x);
	float4 middle = lerp(top, bottom, uv.y);
	return normalize(middle.xyz) * depth;
}

float3 sampleNormal(int2 uv) {
	float2 uv_c = (float2)uv / cb_screenSize;
	float2 uv_v = (float2)(uv + int2(1, 0)) / cb_screenSize;
	float2 uv_h = (float2)(uv + int2(0, 1)) / cb_screenSize;

	float3 p_c = uvToViewPoint(uv_c);
	float3 p_v = uvToViewPoint(uv_v);
	float3 p_h = uvToViewPoint(uv_h);

	float3 to_v = normalize(p_v - p_c);
	float3 to_h = normalize(p_h - p_c);
	float3 normal = normalize(cross(to_v, to_h));

	return normal;
}

float map(float val, float min, float max) { 
	return clamp((val - min) / (max - min), 0, 1);
}

float4 main(VS_OUT ip) : SV_TARGET {
	// float blend = 1.f;
	// float3 edgeColor = float3(0, 0, 0);
	// uint2 screenSize = uint2(cb_screenSize.x, cb_screenSize.y);

	// float2 uv = float2(ip.posH.x / screenSize.x, ip.posH.y / screenSize.y);

	// float pixThis = linearDepth(depthMap[ip.posH.xy]);

	// float table[] = {
	//	0, 1, 2, 1, 0,
	//	1, 2, 3, 2, 1,
	//	2, 3, 0, 3, 2,
	//	1, 2, 3, 2, 1,
	//	0, 1, 2, 1, 0,
	//};

	// float sum = 0;
	// float total = 0;
	// int isize = 5;
	// for (int xx = 0; xx < isize; xx++) {
	//	for (int yy = 0; yy < isize; yy++) {
	//		total += table[yy * isize + xx];
	//		float pixOther = linearDepth(depthMap[ip.posH.xy + int2(xx - (int)(isize/2), yy -
	//(int)(isize/2))]); 		if ((pixThis - pixOther) > 0.15f * pixThis) { 			sum +=
	//table[yy
	//* isize + xx];
	//		}
	//	}
	//}
	// blend = clamp(5*sum / total, 0, 1);

	// return float4(edgeColor, blend);

	//////////////////////////////////////////////////////

	//float depth = linearDepth(depthMap[ip.posH.xy])/100;

	//float3 normal = sampleNormal(ip.posH.xy);
	//float3 normal_adj[4] = { 
	//	sampleNormal(ip.posH.xy + int2(-1, 0)),
	//	sampleNormal(ip.posH.xy + int2(1, 0)), 
	//	sampleNormal(ip.posH.xy + int2(0, -1)),
	//	sampleNormal(ip.posH.xy + int2(0, 1)) 
	//};
	//float sum = 0;
	//for (int i = 0; i < 4; i++) {
	//	float d = dot(normal, normal_adj[i]);
	//	sum += (1 - d);
	//}
	//float a = sum / 4;
	//return float4(0, 0, 0, a);

	/////////////////////////////////////////////////////////

	float depth = linearDepth(depthMap[ip.posH.xy]) / 100;
	float3 normal = sampleNormal(ip.posH.xy);

	float table[] = {
		0, 1, 2, 1, 0,
		1, 2, 3, 2, 1,
		2, 3, 0, 3, 2,
		1, 2, 3, 2, 1,
		0, 1, 2, 1, 0,
	};
	int isize = 5;
	int sum = 0;
	int total = 0;
	for (int xx = 0; xx < isize; xx++) {
		for (int yy = 0; yy < isize; yy++) {
			total += table[yy * isize + xx];
			int2 id = int2(xx - isize / 2, yy - isize / 2);

			float3 adj_normal = sampleNormal(ip.posH.xy + id);
			float d = dot(normal, adj_normal);
			sum += table[yy * isize + xx] *(1-map(d, 0.5, 0.85));
		}
	}
	float far_factor = (1-pow(depth,2))*3;
	float a = clamp(far_factor * sum / total, 0, 1);

	return float4(0, 0, 0, a);
}