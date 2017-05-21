struct ParticleAttributes
{
    float4 position : SV_POSITION;
    uint type : TYPE;
    float age : AGE;
};

struct ParticleVisuals
{
    float4 position : SV_POSITION;
    uint type : TYPE;
    float age : AGE;
    float4 color : COLOR;
};

[maxvertexcount(3)]
void main(
    point ParticleAttributes input[1],
	inout TriangleStream<ParticleVisuals> output
)
{
    float particleSize = 0.025f;
    float4 particleColor = float4(0.0f, 0.0f, 0.0f, 1.0f);

    if (input[0].type == 0)
    {
        particleColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
    }
    else if (input[0].type == 1)
    {
        particleColor = float4(1.0f, 0.0f, 0.0f, 1.0f);

    }
    else if (input[0].type == 2)
    {
        particleColor = float4(0.0f, 1.0f, 0.0f, 1.0f);

    }
    else if (input[0].type == 3)
    {
        particleColor = float4(0.0f, 0.0f, 1.0f, 1.0f);

    }
    
    ParticleVisuals p1, p2, p3;

    p1.position = input[0].position;
    p1.position.x -= particleSize;
    p1.position.y -= particleSize;
    p1.type = input[0].type;
    p1.age = input[0].age;
    p1.color = particleColor;
        
    p2.position = input[0].position;
    p2.position.y += particleSize;
    p2.type = input[0].type;
    p2.age = input[0].age;
    p2.color = particleColor;
        
    p3.position = input[0].position;
    p3.position.x += particleSize;
    p3.position.y -= particleSize;
    p3.type = input[0].type;
    p3.age = input[0].age;
    p3.color = particleColor;

    output.Append(p1);
    output.Append(p2);
    output.Append(p3);
}