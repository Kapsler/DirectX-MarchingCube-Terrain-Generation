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
    float4 lightPos : TEXCOORD0;
};

cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

cbuffer LightMatrixBuffer : register(b1)
{
    matrix lightViewMatrix;
    matrix lightProjectionMatrix;
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
    output.lightPos = mul(output.worldPos, lightViewMatrix);
    output.lightPos = mul(output.lightPos, lightProjectionMatrix);
    //output.lightPos.w = 1.0f;

	return output;
}