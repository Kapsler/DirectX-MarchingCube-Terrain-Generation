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
	inout PointStream<ParticleVisuals> output
)
{
    if (input[0].type == 0)
    {
        ParticleVisuals element;
        element.position = input[0].position;
        element.type = input[0].type;
        element.color = float4(0.0f, 1.0f, 0.0f, 1.0f);
        output.Append(element);
    }

    if (input[0].type == 1)
    {
        ParticleVisuals element;
        element.position = input[0].position;
        element.type = input[0].type;
        element.color = float4(1.0f, 0.0f, 0.0f, 1.0f);
        output.Append(element);
    }     
}