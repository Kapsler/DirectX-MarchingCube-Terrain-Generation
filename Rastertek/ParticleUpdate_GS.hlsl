struct ParticleAttributes
{
    float4 position : SV_POSITION;
    uint type : TYPE;
};

[maxvertexcount(3)]
void main(
    point ParticleAttributes input[1],
	inout PointStream<ParticleAttributes> output
)
{
    ParticleAttributes particle;
    particle.position = input[0].position;
    particle.type = input[0].type;
    output.Append(particle);

    //if (input[0].type == 0)
    //{
    //    ParticleAttributes emitter;
    //    emitter.position = input[0].position;
    //    emitter.type = 0;
    //    output.Append(emitter);
       
    //    ParticleAttributes particle;
    //    particle.position = input[0].position;
    //    particle.position.y += 0.01f;
    //    particle.type = 1;
    //    output.Append(particle);
      
    //}
    
    //if (input[0].type == 1)
    //{
    //    ParticleAttributes particle;
    //    particle.position = input[0].position;
    //    particle.position.y += 0.01f;
    //    particle.type = 1;
    //    output.Append(particle);
    //}

}