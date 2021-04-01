struct VS_OUT {
	float4 posH : SV_POSITION;
};

cbuffer lightInfo : register(b5) {
	float4 ambientColour;
	float4 diffuseColour;
	float4 specularColour;
};
cbuffer screenSizeBuffer : register(b9) { int4 cb_screenSize; };
cbuffer settings : register(b8) {
	float2 cb_SunUV;
	float cb_InitDecay;
	float cb_DistDecay;
	float3 cb_RayColor;
	float cb_MaxDeltaLen;
}

Texture2D<float> depthTexture : register(t0);

SamplerState samp;

static const int NUM_STEPS = 150;
static const float NUM_DELTA = 1.0 / 150.0f;
static const float ASPECT_RATIO = 1280.f/720; // width/height aspect ratio

float sampleDepth(float2 uv) { return float(depthTexture.Sample(samp, uv) >= 0.99999); }

float4 main(VS_OUT ip) : SV_TARGET {
	float2 uv = float2(ip.posH.x / cb_screenSize.x, ip.posH.y / cb_screenSize.y);

	// Find the direction and distance to the sun
	float2 dirToSun = (cb_SunUV - uv);
	float lengthToSun = length(dirToSun);
	dirToSun = normalize(dirToSun);

	// Find the ray delta
	float deltaLen = min(cb_MaxDeltaLen, lengthToSun * NUM_DELTA);
	float2 rayDelta = dirToSun * deltaLen;

	// Each step decay
	//float stepDecay = cb_DistDecay * deltaLen; // this code streches sun to be rectangular
	float screenDeltaLen =
		min(cb_MaxDeltaLen, length((cb_SunUV - uv) * float2(ASPECT_RATIO, 1)) * NUM_DELTA);
	float stepDecay = cb_DistDecay * screenDeltaLen;

	// Initial values
	float2 rayOffset = float2(0.0, 0.0);
	float decay = cb_InitDecay;
	float rayIntensity = 0.0f;

	// Ray march towards the sun
	for (int i = 0; i < NUM_STEPS; i++) {
		// Sample at the current location
		float2 sampPos = uv + rayOffset;
		float fCurIntensity = sampleDepth(sampPos);

		// Sum the intensity taking decay into account
		rayIntensity += fCurIntensity * decay;

		// Advance to the next position
		rayOffset += rayDelta;

		// Update the decay
		decay = saturate(decay - stepDecay);
	}
	// The resultant intensity of the pixel.
	return float4(cb_RayColor * ambientColour.rgb * rayIntensity, 1.f);
}