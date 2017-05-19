struct ParticleAttributes
{
    float4 position : SV_POSITION;
};

[maxvertexcount(3)]
void main(
    point ParticleAttributes input[1],
	inout TriangleStream<ParticleAttributes> output
)
{
    ParticleAttributes element;
	element.position = input[0].position;
	output.Append(element);
}