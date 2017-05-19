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
    ParticleAttributes element;
    element.position = input[0].position;
    element.type = input[0].type;
	output.Append(element);
}