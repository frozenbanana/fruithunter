
struct VS_IN {
	float3 Pos : Position;
	float2 TexCoord : TexCoordinate;
	float3 Normal : Normal;
};

struct VS_OUT {
	float3 PosL : POSITION1;
	float3 PosW : POSITION0;
	float4 PosH : SV_POSITION;
	float2 TexCoord : TEXCOORD;
	float3 Normal : NORMAL;
};
cbuffer cb_world : register(b0) { matrix mWorld, mInvTraWorld; };
cbuffer cb_viewPerspective : register(b1) { matrix mViewPerspective; };
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
cbuffer cb_mapSizes : register(b4) {
	int2 dudvMapSize;
	int2 heightmapSize;
}

Texture2D heightmap : register(t0);
Texture2D dudvMap : register(t1);

float2 clampUV(uint2 uv, uint2 size) {
	return clamp(uv, uint2(0, 0), uint2(size.x - 1, size.y - 1));
}

float4 texSample(Texture2D texMap, uint2 texSize, float2 uv) { 
	float2 mappedUV = uv * (float2)texSize;
	uint2 floorUV = (uint2)mappedUV;
	float2 restUV = mappedUV - floorUV;

	float4 pixThis = texMap[floorUV];
	float4 pixLeft = texMap[clampUV(floorUV + uint2(1, 0),texSize)];
	float4 pixRight = texMap[clampUV(floorUV + uint2(-1, 0),texSize)];
	float4 pixUp = texMap[clampUV(floorUV + uint2(0, 1),texSize)];
	float4 pixDown = texMap[clampUV(floorUV + uint2(0, -1),texSize)];

	float4 horizontal = restUV.x > 0.5 ? (lerp(pixThis, pixLeft, restUV.x * 2 - 1))
									   : (lerp(pixRight, pixThis, restUV.x * 2));
	float4 vertical = restUV.y > 0.5 ? (lerp(pixThis, pixUp, restUV.y * 2 - 1))
									 : (lerp(pixDown, pixThis, restUV.y * 2));
	return lerp(horizontal, vertical, 0.5);
}

VS_OUT main(VS_IN input) {
	VS_OUT output = (VS_OUT)0;

	//water height
	float2 uvTime = ((input.TexCoord*10.f)%1.f) + float2(1, 1) * time.x * timeSpeed;
	float2 uvTime2 = ((input.TexCoord*25.f)%1.f) + float2(1, 1) * time.x * timeSpeed*0.5f;
	float sampledHeight = texSample(heightmap, (uint2)heightmapSize, (uvTime) % 1.f).x;
	float sampledHeight2 = texSample(heightmap, (uint2)heightmapSize, (uvTime2) % 1.f).x;
	input.Pos.y = clamp(length(input.TexCoord * 2 - float2(1, 1)) * tideHeightStrength,
					  tideHeightScaling.x, tideHeightScaling.y) *
				  (sampledHeight + sampledHeight2) / 2.f;

	//transforms
	output.PosL = input.Pos.xyz;
	output.PosW = mul(float4(input.Pos, 1), mWorld).xyz;
	output.PosH = mul(float4(output.PosW, 1), mViewPerspective);
	output.TexCoord = input.TexCoord;
	output.Normal = normalize(mul(float4(input.Normal, 0), mInvTraWorld).xyz);

	return output;
}