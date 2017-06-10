cbuffer MatrixBuffer
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

struct VertexInputType
{
    float4 position : SV_Position;
};

struct PixelInputType
{
    float4 position : SV_Position;
    float4 fixedPointDepth : TEXTURE0;
    float4 linearDepth : TEXCOORD1;
};

PixelInputType main(VertexInputType input)
{
    PixelInputType output;

    input.position.w = 1.0f;

    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);

    output.linearDepth = output.position;

    output.position = mul(output.position, projectionMatrix);

    output.fixedPointDepth = output.position;

	return output;
}