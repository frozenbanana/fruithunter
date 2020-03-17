struct Input {
	float4 position : SV_POSITION;
	float3 color : COLOR;
	float2 uv : TEXCOORD;
};

Texture2D objTexture : TEXTURE : register(t0);
SamplerState objSamplerState : SAMPLER : register(s0);

#define WIDTH 2560.0f
#define HEIGHT 1440.0f

//Calculate luminosity or "brightness" of a color based on given formula
float rgb2luma(float3 rgb)
{
    return sqrt(dot(rgb, float3(0.299f, 0.587f, 0.114f)));
}

//Function to speed up edge walking by returning larger values for later iterations
float Quality(int i)
{
    if(i < 6)
        return 1.0f;
    else if(i < 7)
        return 1.5f;
    else if (i < 11)
        return 2.0f;
    else if (i < 12)
        return 4.0f;
    else
        return 8.0f;
}

//Returns anti-aliasing adjusted rgb color from texure
float3 FXAA(float2 texCoord)
{
    float3 colorCenter = objTexture.Sample(objSamplerState, texCoord).rgb;
    
    //Luma at current pixel
    float lumaCenter = rgb2luma(colorCenter);

    //Luma at surrounding pixels
	float lumaDown = rgb2luma(objTexture.Sample(objSamplerState, texCoord, int2(0, 1)).rgb);
	float lumaUp = rgb2luma(objTexture.Sample(objSamplerState, texCoord, int2(0, -1)).rgb);
	float lumaLeft = rgb2luma(objTexture.Sample(objSamplerState, texCoord, int2(-1, 0)).rgb);
	float lumaRight = rgb2luma(objTexture.Sample(objSamplerState, texCoord, int2(1, 0)).rgb);

    //Find minimum and maximum luma and calculate difference   
    float lumaMin = min(lumaCenter, min(min(lumaDown, lumaUp), min(lumaLeft, lumaRight)));
    float lumaMax = max(lumaCenter, max(max(lumaDown, lumaUp), max(lumaLeft, lumaRight)));
    float lumaRange = lumaMax - lumaMin;

    //If luma difference is lower than threshold, or in a dark area, return without anti-aliasing
    if (lumaRange < max(0.0312f, lumaMax * 0.125f))
        return colorCenter;

    //Luma at corner pixels
	float lumaDownLeft = rgb2luma(objTexture.Sample(objSamplerState, texCoord, int2(-1, 1)).rgb);
	float lumaDownRight = rgb2luma(objTexture.Sample(objSamplerState, texCoord, int2(1, 1)).rgb);
	float lumaUpLeft = rgb2luma(objTexture.Sample(objSamplerState, texCoord, int2(-1, -1)).rgb);
	float lumaUpRight = rgb2luma(objTexture.Sample(objSamplerState, texCoord, int2(1, -1)).rgb);

    //Directional lumas
    float lumaDownUp = lumaDown + lumaUp;
    float lumaLeftRight = lumaLeft + lumaRight;
    float lumaLeftCorners = lumaUpLeft + lumaDownLeft;
    float lumaRightCorners = lumaUpRight + lumaDownRight;
    float lumaUpCorners = lumaUpLeft + lumaUpRight;
    float lumaDownCorners = lumaDownLeft + lumaDownRight;

    //Determine if the edge is more horizontal or vertical
    float edgeHorizontal = abs(-2.0f * lumaLeft + lumaLeftCorners) + abs(-2.0f * lumaCenter + lumaDownUp) * 2.0f + abs(-2.0f * lumaRight + lumaRightCorners);
    float edgeVertical = abs(-2.0f * lumaUp + lumaUpCorners) + abs(-2.0f * lumaCenter + lumaLeftRight) * 2.0f + abs(-2.0f * lumaDown + lumaDownCorners);
    bool isHorizontal = edgeHorizontal > edgeVertical;

    //Select correct lumas based on direction and calculate gradients
    float lumaDR = isHorizontal ? lumaDown : lumaRight;
    float lumaUL = isHorizontal ? lumaUp : lumaLeft;
    float gradientDR = lumaDR - lumaCenter;
    float gradientUL = lumaUL - lumaCenter;

    //Determine the steepest direction and calculate scaled gradient
    bool isULSteepest = abs(gradientDR) < abs(gradientUL);
    float gradientScaled = 0.25 * max(abs(gradientDR), abs(gradientUL));

    //Choose step size based on screen size and edge direction
    float stepLength = isHorizontal ? 1.0f / HEIGHT : 1.0f / WIDTH;
    
    //Get average lumas and correct directions
    float lumaLocalAverage = 0.0f;
    if (isULSteepest)
    {
        stepLength = -stepLength;
        lumaLocalAverage = 0.5 * (lumaUL + lumaCenter);
    }
    else
    {
        lumaLocalAverage = 0.5 * (lumaDR + lumaCenter);
    }

	//Adjust texture coordinate half a pixel based on edge direction
    float2 currentCoord = texCoord;
    if (isHorizontal)
        currentCoord.y += stepLength * 0.5;
    else
        currentCoord.x += stepLength * 0.5;

	//Get initial offsets to explore along the edge
    float2 offset = isHorizontal ? float2(1.0f / WIDTH, 0.0f) : float2(0.0, 1.0f / HEIGHT);
    float2 coord1 = currentCoord - offset;
    float2 coord2 = currentCoord + offset;

	//Read the lumas at both ends and compare to original luma
	float lumaEnd1 = rgb2luma(objTexture.Sample(objSamplerState, coord1).rgb);
	float lumaEnd2 = rgb2luma(objTexture.Sample(objSamplerState, coord2).rgb);
    lumaEnd1 -= lumaLocalAverage;
    lumaEnd2 -= lumaLocalAverage;

	//If the delta is larger than local gradient the end has been reached
    bool reached1 = abs(lumaEnd1) > gradientScaled;
    bool reached2 = abs(lumaEnd2) > gradientScaled;
    bool reachedBoth = reached1 && reached2;

	//Continue exploring if the edge was not reached
    if (!reached1)
        coord1 -= offset;
    if (!reached2)
        coord2 += offset;

	//Continue doing the steps above until both ends have been reached or distance to center is large enough
    for (int i = 2; !reachedBoth && i < 12; i++)
    {
        if (!reached1)
        {
			lumaEnd1 = rgb2luma(objTexture.Sample(objSamplerState, coord1).rgb);
            lumaEnd1 -= lumaLocalAverage;
        }
        if (!reached2)
        {
			lumaEnd2 = rgb2luma(objTexture.Sample(objSamplerState, coord2).rgb);
            lumaEnd2 -= lumaLocalAverage;
        }

        reached1 = abs(lumaEnd1) > gradientScaled;
        reached2 = abs(lumaEnd2) > gradientScaled;
        reachedBoth = reached1 && reached2;

		//Uses the "Quality" function to take larger steps for long edges
        if (!reached1)
            coord1 -= offset * Quality(i);
        if (!reached2)
            coord2 += offset * Quality(i);
    }
       
	//Determine the distance to the endpoints of the edge   
    float distance1 = isHorizontal ? abs(texCoord.x - coord1.x) : abs(texCoord.y - coord1.y);
    float distance2 = isHorizontal ? abs(coord2.x - texCoord.x) : abs(coord2.y - texCoord.y);

	//Compare the distances
    bool isDirection1 = distance1 < distance2;
    float distanceFinal = min(distance1, distance2);
	
	//Full length of the edge
    float edgeThickness = distance1 + distance2;

	//Use the information above to set the estimated offset for the texture coordinate
    float pixelOffset = -distanceFinal / edgeThickness + 0.5f;

	//Additional test to ensure we have not stepped beyond the edge
    bool isLumaCenterSmaller = lumaCenter < lumaLocalAverage;
    bool correctVariation = ((isDirection1 ? lumaEnd1 : lumaEnd2) < 0.0f) != isLumaCenterSmaller;
    float finalOffset = correctVariation ? pixelOffset : 0.0f;

	//Subpixel anti-aliasing to handle thin lines and small objects
	//Calculate weighted average luma around the center pixel
    float lumaAverage = (1.0f / 12.0f) * (2.0f * (lumaDownUp + lumaLeftRight) + lumaLeftCorners + lumaRightCorners);

	//Determine offset by comparing center luma with luma of surrounding pixels
	//Big differences in luma give a larger offset while smaller differences in luma gives a smaller offset
    float subPixelOffset1 = clamp(abs(lumaAverage - lumaCenter) / lumaRange, 0.0f, 1.0f);
    float subPixelOffset2 = (-2.0f * subPixelOffset1 + 3.0f) * subPixelOffset1 * subPixelOffset1;
    float subPixelOffsetFinal = subPixelOffset2 * subPixelOffset2 * 0.75f;

	//Sets the final offset to the largest one between normal and subpixel anti-aliasing
    finalOffset = max(finalOffset, subPixelOffsetFinal);

	//Get the final coordinate to read from the texture using the direction, offsets and pixel size
    float2 finalCoord = texCoord;
    if (isHorizontal)
        finalCoord.y += finalOffset * stepLength;
    else
        finalCoord.x += finalOffset * stepLength;
	
	//Return the final interpolated color from the texutre
	return objTexture.Sample(objSamplerState, finalCoord).rgb;
}

float4 main(Input input) : SV_Target {
    return float4(FXAA(input.uv), 1.0f);
};


