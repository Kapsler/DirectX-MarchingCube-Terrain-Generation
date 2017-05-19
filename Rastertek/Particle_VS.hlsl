struct ParticleAttributes
{
    float4 position : SV_POSITION;
};

ParticleAttributes main(ParticleAttributes input)
{
    ParticleAttributes output;

    output = input;

    return output;
}