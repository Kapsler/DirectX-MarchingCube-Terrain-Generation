struct ParticleAttributes
{
    float4 position : SV_POSITION;
    uint type : TYPE;
};

ParticleAttributes main(ParticleAttributes input)
{
    ParticleAttributes output;

    output.position = input.position;
    output.type = input.type;

    return output;
}