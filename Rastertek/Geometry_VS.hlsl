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
    float4 worldPos : POSITION0;
    float4 color : COLOR;
    float4 normal : NORMAL;
    float4 lightViewPos : TEXCOORD0;
    float4 lightViewPosVSM : TEXCOORD1;
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
    //output.position = mul(output.position, projectionMatrix);

    output.color = input.color;

    output.normal = input.normal;

    output.lightViewPosVSM = mul(output.worldPos, lightViewMatrix);
    output.lightViewPos = output.lightViewPosVSM;
    //output.lightViewPos = mul(output.lightViewPosVSM, lightProjectionMatrix);

	return output;
}