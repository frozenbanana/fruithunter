struct VS_Input {
	float3 position : Position;
	float rotation : RotationY;
	float height : Height;
};

struct VS_Output {
	float3 position : POSITION0;
	float rotation : RotationY;
	float height : Height;
};

VS_Output main(VS_Input input)
{
	VS_Output output;
	output.position = input.position;
	output.rotation = input.rotation;
	output.height = input.height;
	return output;
}