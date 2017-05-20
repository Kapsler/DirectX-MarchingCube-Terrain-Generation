struct ParticleAttributes
{
    float4 position : SV_POSITION;
    uint type : TYPE;
};

cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

float4 GetFinalPosition(float4 pos)
{
    float4 result;

    result = mul(pos, worldMatrix);
    result = mul(result, viewMatrix);
    result = mul(result, projectionMatrix);

    return result;
}

ParticleAttributes main(ParticleAttributes input)
{
    ParticleAttributes output;

    output.position = GetFinalPosition(input.position);
    output.type = input.type;

    return output;
}