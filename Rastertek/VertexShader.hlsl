cbuffer MatrixBuffer
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

struct VertexInputType
{
    float3 position : POSITION;
    float4 color : COLOR;
};

struct PixelInputType
{
    float4 position : SV_Position;
    float4 color : COLOR;
};

PixelInputType main( VertexInputType input )
{
	PixelInputType output;

	// Change the position vector to be 4 units for proper matrix calculations.
    output.position = float4(input.position, 1.0f);

	// Store the input color for the pixel shader to use.
	output.color = input.color;

	return output;
}