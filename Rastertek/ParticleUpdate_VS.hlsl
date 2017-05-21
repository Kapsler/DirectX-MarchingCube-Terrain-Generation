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

ParticleAttributes main(ParticleAttributes input)
{
    ParticleAttributes output;

    output.position = input.position;
    output.type = input.type;

    return output;
}