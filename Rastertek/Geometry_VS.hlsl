struct VertexInput
{
    float4 position : SV_POSITION;
    float4 worldPos : POSITION;
    float4 color : COLOR;
    float4 normal : NORMAL;
};

struct PixelInput
{
    float4 position : SV_POSITION;
    float4 worldPos : POSITION;
    float4 color : COLOR;
    float4 normal : NORMAL;
};

cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

PixelInput main(VertexInput input)
{
    PixelInput output;

    output.position = mul(input.position, worldMatrix);
    output.worldPos = output.position;
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    output.color = input.color;
    output.normal = input.normal;

	return output;
}