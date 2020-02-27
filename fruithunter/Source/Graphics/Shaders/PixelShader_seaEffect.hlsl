
struct PS_IN {
	float3 PosL : POSITION1;
	float3 PosW : POSITION0;
	float4 PosH : SV_POSITION;
	float2 TexCoord : TEXCOORD;
	float3 Normal : NORMAL;
};

SamplerState aSampler;
Texture2D depthMap : register(t7);
Texture2D dudvMap : register(t1);

cbuffer cb_time : register(b2) { float4 time; }
cbuffer cb_properties : register(b3) {
	float distortionStrength;
	float whiteDepthDifferenceThreshold;
	float timeSpeed;
	int waterShadingLevels;
	float depthDifferenceStrength;
	float3 color_edge;
	float4 color_shallow;
	float4 color_deep;
	float2 heightThreshold_edge;
	float2 tideHeightScaling;
	float tideHeightStrength;
	float3 filler;
}
cbuffer cb_screenSize : register(b5) { int4 screenSize2; }

float linearDepth(float depthSample) {
	const float zNear = 0.025f;
	const float zFar = 100.f;
	depthSample = 2.0 * depthSample - 1.0;
	float zLinear = 2.0 * zNear * zFar / (zFar + zNear - depthSample * (zFar - zNear));
	return zLinear;
}

float clampBetweenIntervals(float v, int intervals) { return floor(v * intervals) / intervals; }

float2 clampUV(uint2 uv, uint2 size) {
	return clamp(uv, uint2(0, 0), uint2(size.x - 1, size.y - 1));
}
float texDepthSample(Texture2D texMap, uint2 texSize, float2 uv) {
	float2 mappedUV = uv * texSize;
	uint2 floorUV = (uint2)mappedUV;
	float2 restUV = mappedUV - floorUV;

	float pixThis = texMap[floorUV].x;
	float pixLeft = texMap[clampUV(floorUV + uint2(1, 0), texSize)].x;
	float pixRight = texMap[clampUV(floorUV + uint2(-1, 0), texSize)].x;
	float pixUp = texMap[clampUV(floorUV + uint2(0, 1), texSize)].x;
	float pixDown = texMap[clampUV(floorUV + uint2(0, -1), texSize)].x;

	float threshold = 0.001;
	if (abs(pixThis - pixLeft) > threshold)
		pixLeft = pixThis;
	if (abs(pixThis - pixRight) > threshold)
		pixRight = pixThis;
	if (abs(pixThis - pixUp) > threshold)
		pixUp = pixThis;
	if (abs(pixThis - pixDown) > threshold)
		pixDown = pixThis;

	float horizontal = restUV.x > 0.5 ? (lerp(pixThis, pixLeft, restUV.x * 2 - 1))
									  : (lerp(pixRight, pixThis, restUV.x * 2));
	float vertical = restUV.y > 0.5 ? (lerp(pixThis, pixUp, restUV.y * 2 - 1))
									: (lerp(pixDown, pixThis, restUV.y * 2));
	return lerp(horizontal, vertical, 0.5);
}

float4 main(PS_IN ip) : SV_TARGET {
	uint2 screenSize = (uint2)screenSize2.xy;

	float2 mappedUV = float2((ip.PosH.x / screenSize.x), (ip.PosH.y / screenSize.y));

	// water depth
	float currentDepth = ip.PosH.z; // maybe use .w
	float currentDepth_World = linearDepth(currentDepth);
	// distortion
	float distortionStrengthWithDepth = distortionStrength * (1.f / currentDepth_World);
	float2 texCoord = ip.TexCoord;
	float2 uvOffset1 =
		(dudvMap.Sample(aSampler, texCoord + float2(1, 0) * time.x * timeSpeed) * 2 - 1).xy *
		distortionStrengthWithDepth;
	float2 uvOffset2 = (dudvMap.Sample(aSampler, texCoord.yx * float2(-1, 1) + float2(0, 1) * time.x * timeSpeed) *
							   2 -
						   1).xy *
					   distortionStrengthWithDepth;
	float2 uvOffset = uvOffset1 + uvOffset2;
	// non water depth
	float sampledDepth = texDepthSample(depthMap, screenSize, mappedUV + uvOffset).x;
	if (sampledDepth < currentDepth)
		sampledDepth = texDepthSample(depthMap, screenSize, mappedUV)
						   .x; // resample depth if the distorted sampling returns invalid value
	float sampledDepth_World = linearDepth(sampledDepth);
	// height color
	float height = ip.PosL.y;
	float2 heightThreshold = heightThreshold_edge.xy;
	// interpolate color
	float depthValue =
		clamp((sampledDepth_World - currentDepth_World) * depthDifferenceStrength, 0, 1);
	depthValue = clampBetweenIntervals(depthValue, waterShadingLevels);
	float3 finalColor;
	if (depthValue < whiteDepthDifferenceThreshold || (height > heightThreshold.y))
		finalColor = color_edge.rgb; // edge coloring
	else {
		float3 depthColor =
			lerp(color_shallow.rgb, color_deep.rgb, float3(1, 1, 1) * depthValue); // depth
																				   // coloring
		float3 heightColor = lerp(color_deep, color_shallow, (height / heightThreshold.x)).rgb;
		float3 seaColor = lerp(depthColor, heightColor, 0.5);
		if (height > heightThreshold.x)
			finalColor = lerp(seaColor, color_edge.rgb,
				(height - heightThreshold.x) / (heightThreshold.y - heightThreshold.x));
		else
			finalColor = seaColor;
	}
	// draw color
	return float4(finalColor, 1.f); // final color
}