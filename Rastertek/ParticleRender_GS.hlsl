struct ParticleAttributes
{
    float4 position : SV_POSITION;
    uint type : TYPE;
};

struct ParticleVisuals
{
    float4 position : SV_POSITION;
    uint type : TYPE;
    float4 color : COLOR;
};

[maxvertexcount(3)]
void main(
    point ParticleAttributes input[1],
	inout TriangleStream<ParticleVisuals> output
)
{
    float offset = 1.0f;
    float4 particleColor;

    if (input[0].type == 0)
    {
        particleColor = float4(0.0f, 1.0f, 0.0f, 1.0f);
    }

    if (input[0].type == 1)
    {
        particleColor = float4(1.0f, 0.0f, 0.0f, 1.0f);

    }  
    
    ParticleVisuals p1, p2, p3;

    p1.position = input[0].position;
    p1.type = input[0].type;
    p1.color = particleColor;
        
    p2.position = input[0].position;
    p2.position.x = p2.position.x + offset * 0.5f;
    p2.position.y = p2.position.y + offset;
    p2.type = input[0].type;
    p2.color = particleColor;
        
    p3.position = input[0].position;
    p3.position.x = p3.position.x + offset;
    p3.type = input[0].type;
    p3.color = particleColor;

    output.Append(p1);
    output.Append(p2);
    output.Append(p3);
}