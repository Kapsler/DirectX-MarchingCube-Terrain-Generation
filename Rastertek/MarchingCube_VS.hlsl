cbuffer MatrixBuffer
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

struct VertexInputType
{
    float3 position : SV_POSITION;
    float4 color : COLOR;
};

struct GSInputType
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

GSInputType main(VertexInputType input)
{
    GSInputType output;

	// Change the position vector to be 4 units for proper matrix calculations.
    output.position = float4(input.position.xyz, 1.0f);

	// Store the input color for the pixel shader to use.
	output.color = input.color;

	return output;
}