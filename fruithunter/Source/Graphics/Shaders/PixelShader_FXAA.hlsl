// https://github.com/kosua20/Rendu/blob/master/resources/common/shaders/screens/fxaa.frag
// Settings for FXAA.
#define EDGE_THRESHOLD_MIN 0.0312
#define EDGE_THRESHOLD_MAX 0.125
#define QUALITY(q) ((q) < 5 ? 1.0 : ((q) > 5 ? ((q) < 10 ? 2.0 : ((q) < 11 ? 4.0 : 8.0)) : 1.5))
#define ITERATIONS 12
#define SUBPIXEL_QUALITY 0.75

struct Input {
	float4 posH : SV_POSITION;
	float2 uv : TexCoord;
};

Texture2D objTexture : TEXTURE : register(t1);
SamplerState objSamplerState : SAMPLER : register(s0);
cbuffer screenSizeConstantBuffer : register(b8){ int4 screenSize; };

float RGBToLuma(float3 colour) {
	colour.rgb = sqrt(colour.rgb);
	float luma = dot(colour.rgb, float3(0.299f, 0.587f, 0.114));
	return luma;
}

float4 main(Input input) : SV_TARGET {
	float4 fragColour = float4(1.0f, 1.0f, 0.0f, 1.0f);

	float2 inverseScreenSize = float2(
		1.f / screenSize.x, 1.f / screenSize.y);

	// Get UV coordinates
	float2 texCoords = input.uv;

	// Get current pixel colour
	float3 colorCenter = objTexture.Sample(objSamplerState, texCoords).rgb;

	// Get luma at current pixel
	float lumaCenter = RGBToLuma(colorCenter);

	// Get Luma at neightbours
	float lumaDown = RGBToLuma(
		objTexture.Sample(objSamplerState, texCoords + float2(0.f, -inverseScreenSize.y)).rgb);
	float lumaUp = RGBToLuma(
		objTexture.Sample(objSamplerState, texCoords + float2(0.f, inverseScreenSize.y)).rgb);
	float lumaLeft = RGBToLuma(
		objTexture.Sample(objSamplerState, texCoords + float2(-inverseScreenSize.x, 0.f)).rgb);
	float lumaRight = RGBToLuma(
		objTexture.Sample(objSamplerState, texCoords + float2(inverseScreenSize.x, 0.f)).rgb);

	// Find the maximum and minimum luma around the current fragment.
	float lumaMin = min(lumaCenter, min(min(lumaDown, lumaUp), min(lumaLeft, lumaRight)));
	float lumaMax = max(lumaCenter, max(max(lumaDown, lumaUp), max(lumaLeft, lumaRight)));

	// Compute the delta.
	float lumaRange = lumaMax - lumaMin;

	// If the luma variation is lower that a threshold (or if we are in a really dark area), we are
	// not on an edge, don't perform any AA.
	if (lumaRange < max(EDGE_THRESHOLD_MIN, lumaMax * EDGE_THRESHOLD_MAX)) {
		fragColour.rgb = colorCenter;
		return fragColour;
	}

	// Query the 4 remaining corners lumas.
	float lumaDownLeft = RGBToLuma(
		objTexture
			.Sample(objSamplerState, texCoords + float2(-inverseScreenSize.x, -inverseScreenSize.y))
			.rgb);
	float lumaUpRight = RGBToLuma(
		objTexture
			.Sample(objSamplerState, texCoords + float2(inverseScreenSize.x, inverseScreenSize.y))
			.rgb);
	float lumaUpLeft = RGBToLuma(
		objTexture
			.Sample(objSamplerState, texCoords + float2(-inverseScreenSize.x, inverseScreenSize.y))
			.rgb);
	float lumaDownRight = RGBToLuma(
		objTexture
			.Sample(objSamplerState, texCoords + float2(inverseScreenSize.x, -inverseScreenSize.y))
			.rgb);

	// Combine the four edges lumas (using intermediary variables for future computations with the
	// same values).
	float lumaDownUp = lumaDown + lumaUp;
	float lumaLeftRight = lumaLeft + lumaRight;

	// Same for corners
	float lumaLeftCorners = lumaDownLeft + lumaUpLeft;
	float lumaDownCorners = lumaDownLeft + lumaDownRight;
	float lumaRightCorners = lumaDownRight + lumaUpRight;
	float lumaUpCorners = lumaUpRight + lumaUpLeft;

	// Compute an estimation of the gradient along the horizontal and vertical axis.
	float edgeHorizontal = abs(-2.0 * lumaLeft + lumaLeftCorners) +
						   abs(-2.0 * lumaCenter + lumaDownUp) * 2.0 +
						   abs(-2.0 * lumaRight + lumaRightCorners);
	float edgeVertical = abs(-2.0 * lumaUp + lumaUpCorners) +
						 abs(-2.0 * lumaCenter + lumaLeftRight) * 2.0 +
						 abs(-2.0 * lumaDown + lumaDownCorners);

	// Is the local edge horizontal or vertical ?
	bool isHorizontal = (edgeHorizontal >= edgeVertical);

	// Choose the step size (one pixel) accordingly.
	float stepLength = isHorizontal ? inverseScreenSize.y : inverseScreenSize.x;

	// Select the two neighboring texels lumas in the opposite direction to the local edge.
	float luma1 = isHorizontal ? lumaDown : lumaLeft;
	float luma2 = isHorizontal ? lumaUp : lumaRight;

	// Compute gradients in this direction.
	float gradient1 = luma1 - lumaCenter;
	float gradient2 = luma2 - lumaCenter;

	// Which direction is the steepest ?
	bool is1Steepest = abs(gradient1) >= abs(gradient2);

	// Gradient in the corresponding direction, normalized.
	float gradientScaled = 0.25 * max(abs(gradient1), abs(gradient2));

	// Average luma in the correct direction.
	float lumaLocalAverage = 0.0;
	if (is1Steepest) {
		// Switch the direction
		stepLength = -stepLength;
		lumaLocalAverage = 0.5 * (luma1 + lumaCenter);
	}
	else {
		lumaLocalAverage = 0.5 * (luma2 + lumaCenter);
	}

	// Shift UV in the correct direction by half a pixel.
	float2 currentUv = texCoords;
	if (isHorizontal) {
		currentUv.y += stepLength * 0.5;
	}
	else {
		currentUv.x += stepLength * 0.5;
	}

	// Compute offset (for each iteration step) in the right direction.
	float2 offset =
		isHorizontal ? float2(inverseScreenSize.x, 0.0) : float2(0.0, inverseScreenSize.y);
	// Compute UVs to explore on each side of the edge, orthogonally. The QUALITY allows us to step
	// faster.
	float2 uv1 = currentUv - offset * QUALITY(0);
	float2 uv2 = currentUv + offset * QUALITY(0);

	// Read the lumas at both current extremities of the exploration segment, and compute the delta
	// wrt to the local average luma.
	float lumaEnd1 = RGBToLuma(objTexture.Sample(objSamplerState, uv1).rgb);
	float lumaEnd2 = RGBToLuma(objTexture.Sample(objSamplerState, uv2).rgb);
	lumaEnd1 -= lumaLocalAverage;
	lumaEnd2 -= lumaLocalAverage;

	// If the luma deltas at the current extremities is larger than the local gradient, we have
	// reached the side of the edge.
	bool reached1 = abs(lumaEnd1) >= gradientScaled;
	bool reached2 = abs(lumaEnd2) >= gradientScaled;
	bool reachedBoth = reached1 && reached2;

	// If the side is not reached, we continue to explore in this direction.
	if (!reached1) {
		uv1 -= offset * QUALITY(1);
	}
	if (!reached2) {
		uv2 += offset * QUALITY(1);
	}

	// If both sides have not been reached, continue to explore.
	if (!reachedBoth) {

		for (int i = 2; i < ITERATIONS; i++) {
			// If needed, read luma in 1st direction, compute delta.
			if (!reached1) {
				lumaEnd1 = RGBToLuma(objTexture.Sample(objSamplerState, uv1).rgb);
				lumaEnd1 = lumaEnd1 - lumaLocalAverage;
			}
			// If needed, read luma in opposite direction, compute delta.
			if (!reached2) {
				lumaEnd2 = RGBToLuma(objTexture.Sample(objSamplerState, uv2).rgb);
				lumaEnd2 = lumaEnd2 - lumaLocalAverage;
			}
			// If the luma deltas at the current extremities is larger than the local gradient, we
			// have reached the side of the edge.
			reached1 = abs(lumaEnd1) >= gradientScaled;
			reached2 = abs(lumaEnd2) >= gradientScaled;
			reachedBoth = reached1 && reached2;

			// If the side is not reached, we continue to explore in this direction, with a variable
			// quality.
			if (!reached1) {
				uv1 -= offset * QUALITY(i);
			}
			if (!reached2) {
				uv2 += offset * QUALITY(i);
			}

			// If both sides have been reached, stop the exploration.
			if (reachedBoth) {
				break;
			}
		}
	}

	// Compute the distances to each side edge of the edge (!).
	float distance1 = isHorizontal ? (texCoords.x - uv1.x) : (texCoords.y - uv1.y);
	float distance2 = isHorizontal ? (uv2.x - texCoords.x) : (uv2.y - texCoords.y);

	// In which direction is the side of the edge closer ?
	bool isDirection1 = distance1 < distance2;
	float distanceFinal = min(distance1, distance2);

	// Thickness of the edge.
	float edgeThickness = (distance1 + distance2);

	// Is the luma at center smaller than the local average ?
	bool isLumaCenterSmaller = lumaCenter < lumaLocalAverage;

	// If the luma at center is smaller than at its neighbour, the delta luma at each end should be
	// positive (same variation).
	bool correctVariation1 = (lumaEnd1 < 0.0) != isLumaCenterSmaller;
	bool correctVariation2 = (lumaEnd2 < 0.0) != isLumaCenterSmaller;

	// Only keep the result in the direction of the closer side of the edge.
	bool correctVariation = isDirection1 ? correctVariation1 : correctVariation2;

	// UV offset: read in the direction of the closest side of the edge.
	float pixelOffset = -distanceFinal / edgeThickness + 0.5;

	// If the luma variation is incorrect, do not offset.
	float finalOffset = correctVariation ? pixelOffset : 0.0;

	// Sub-pixel shifting
	// Full weighted average of the luma over the 3x3 neighborhood.
	float lumaAverage =
		(1.0 / 12.0) * (2.0 * (lumaDownUp + lumaLeftRight) + lumaLeftCorners + lumaRightCorners);
	// Ratio of the delta between the global average and the center luma, over the luma range in the
	// 3x3 neighborhood.
	float subPixelOffset1 = clamp(abs(lumaAverage - lumaCenter) / lumaRange, 0.0, 1.0);
	float subPixelOffset2 = (-2.0 * subPixelOffset1 + 3.0) * subPixelOffset1 * subPixelOffset1;
	// Compute a sub-pixel offset based on this delta.
	float subPixelOffsetFinal = subPixelOffset2 * subPixelOffset2 * SUBPIXEL_QUALITY;

	// Pick the biggest of the two offsets.
	finalOffset = max(finalOffset, subPixelOffsetFinal);

	// Compute the final UV coordinates.
	float2 finalUv = texCoords;
	if (isHorizontal) {
		finalUv.y += finalOffset * stepLength;
	}
	else {
		finalUv.x += finalOffset * stepLength;
	}

	// Read the color at the new UV coordinates, and use it.
	float3 finalColor = objTexture.Sample(objSamplerState, finalUv).rgb;
	fragColour.rgb = finalColor;

	return fragColour;
}