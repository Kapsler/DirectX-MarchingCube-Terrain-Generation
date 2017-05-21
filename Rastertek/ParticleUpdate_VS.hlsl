struct ParticleAttributes
{
    float4 position : SV_POSITION;
    uint type : TYPE;
    float age : AGE;
};

ParticleAttributes main(ParticleAttributes input)
{
    ParticleAttributes output;

    output.position = input.position;
    output.type = input.type;
    output.age = input.age;

    return output;
}