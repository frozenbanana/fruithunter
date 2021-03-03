struct VS_OUT {
	float3 PosV : POSITION0;
	float rotation : RotationY;
	float height : Height;
};

struct GSOutput {
	float4 PosW : POSITION0;
	float4 PosH : SV_POSITION;
	float4 Color : Color;
	float2 uv : UV;
	float4 ShadowPosH : POSITION2;
};

cbuffer strawSettings : register(b5) { 
	float cb_baseWidth;
	float2 cb_heightRange;
	float cb_noiseInterval;
	float4 cb_color_top; 
	float4 cb_color_bottom; 
}
cbuffer strawAnimation : register(b6) {
	float cb_time;
	float cb_speed;
	float cb_noiseAnimInterval;
	float cb_offsetStrength;
}

cbuffer cb_world : register(b0) { matrix mWorld, mInvTraWorld; };
cbuffer cb_viewPerspective : register(b1) { matrix mView, mPerspective, mViewPerspective; };
cbuffer cb_shadowVPT : register(b4) { matrix mShadowVPT; };

cbuffer noiseTexInfo : register(b9) {
	float4 cb_noiseTexSize;
}
Texture2D texture_noise : register(t0);

float2 clampUV(uint2 uv, uint2 size) {
	return clamp(uv, uint2(0, 0), uint2(size.x - 1, size.y - 1));
}

float4 texSample(Texture2D texMap, uint2 texSize, float2 uv) {
	float2 mappedUV = uv * (float2)texSize;
	uint2 floorUV = (uint2)mappedUV;
	float2 restUV = mappedUV - floorUV;

	float4 pixThis = texMap[floorUV];
	float4 pixLeft = texMap[clampUV(floorUV + uint2(1, 0), texSize)];
	float4 pixRight = texMap[clampUV(floorUV + uint2(-1, 0), texSize)];
	float4 pixUp = texMap[clampUV(floorUV + uint2(0, 1), texSize)];
	float4 pixDown = texMap[clampUV(floorUV + uint2(0, -1), texSize)];

	float4 horizontal = restUV.x > 0.5 ? (lerp(pixThis, pixLeft, restUV.x * 2 - 1))
									   : (lerp(pixRight, pixThis, restUV.x * 2));
	float4 vertical = restUV.y > 0.5 ? (lerp(pixThis, pixUp, restUV.y * 2 - 1))
									 : (lerp(pixDown, pixThis, restUV.y * 2));
	return lerp(horizontal, vertical, 0.5);
}

[maxvertexcount(3)] 
void main(point VS_OUT input[1], inout TriangleStream<GSOutput> output) {

	float3 posW = mul(float4(input[0].PosV, 1), mWorld);
	float rotation = input[0].rotation;
	float height = cb_heightRange.x + input[0].height * (cb_heightRange.y - cb_heightRange.x);
	float width = cb_baseWidth;
	float2 tex_noise_size = cb_noiseTexSize.xy;

	// grass height variation (world position dependent)
	float2 noiseUV = (float2(posW.x, posW.z) * cb_noiseInterval) % tex_noise_size;
	float noise = texture_noise[(int2)noiseUV].r;
	height *= noise;

	// animation
	float3 windDir = normalize(float3(1, 0, 1));
	float2 noiseAnimUV =
		(float2(posW.x, posW.z) * cb_noiseAnimInterval + float2(1, 1) * cb_time * cb_speed) /
		tex_noise_size;
	float windFactor = texSample(texture_noise, tex_noise_size, noiseAnimUV % 1).r;
	windFactor = windFactor * 2 - 1; // remap to [-1,1]
	float3 offset = windDir * windFactor * cb_offsetStrength;

	// create vertices
	float3 up = float3(0, 1, 0);
	float3 dir = float3(cos(rotation), 0, sin(rotation));

	float3 positions[3];
	float2 uvs[3];
	positions[0] = posW + up * 0.f + dir * width; // point base 1
	positions[1] = posW + up * 0.f - dir * width; // point base 2
	positions[2] = posW + up * height + dir * 0.f + offset; // point top

	// set output
	GSOutput e;
	for (int i = 0; i < 3; i++) {
		e.PosW = float4(positions[i], 1);
		e.PosH = mul(float4(positions[i], 1), mViewPerspective);
		e.ShadowPosH = mul(float4(positions[i], 1), mShadowVPT);
		float3 baseColor = (i % 3 == 2) ? cb_color_top : cb_color_bottom;
		e.Color = float4(baseColor, 1);
		e.uv = float2(0, 0);
		output.Append(e);
	}
}