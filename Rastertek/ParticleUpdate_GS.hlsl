struct ParticleAttributes
{
    float4 position : SV_POSITION;
    uint type : TYPE;
    float age : AGE;
};

cbuffer ParticleInfosBuffer : register(b0)
{
    float deltaTime;
    float3 padding;
};

[maxvertexcount(4)]
void main(
    point ParticleAttributes input[1],
	inout PointStream<ParticleAttributes> output
)
{
    float movementSpeed = 0.001f * deltaTime;
    float lifeTimeInMilliseconds = 2000.0f;

    //Emitter
    if (input[0].type == 0)
    {
        ParticleAttributes emitter;
        emitter.position = input[0].position;
        emitter.type = 0;
        emitter.age = 0;
        output.Append(emitter);
       
        ParticleAttributes particle;
        particle.position = input[0].position;
        particle.position.x += movementSpeed;
        particle.type = 1;
        particle.age = 0;
        output.Append(particle);
        
        particle.position = input[0].position;
        particle.position.y += movementSpeed;
        particle.type = 2;
        particle.age = 0;
        output.Append(particle);

        particle.position = input[0].position;
        particle.position.x -= movementSpeed;
        particle.type = 3;
        particle.age = 0;
        output.Append(particle);
    }
    else
    {
        float3 moveDir = float3(0.0f, 0.0f, 0.0f);

        //Type 1
        if (input[0].type == 1)
        {
            moveDir = float3(1.0f, cos(input[0].age * 0.01f), sin(input[0].age * 0.01f));   
        }
    
        //Type 2
        if (input[0].type == 2)
        {
            moveDir = float3(cos(input[0].age * 0.01f), 1.0f, sin(input[0].age * 0.01f));
        }
    
        //Type 3
        if (input[0].type == 3)
        {
            moveDir = float3(-1.0f, cos(input[0].age * 0.01f), sin(input[0].age * 0.01f));
        }

        if (input[0].age < lifeTimeInMilliseconds)
        {
            ParticleAttributes particle;
            particle.position = input[0].position;
            particle.position.xyz += moveDir * movementSpeed;
            particle.type = input[0].type;
            particle.age = input[0].age + deltaTime;
            output.Append(particle);
        }
    }
   
    

}