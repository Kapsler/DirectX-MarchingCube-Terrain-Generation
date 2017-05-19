struct ParticleVisuals
{
    float4 position : SV_POSITION;
    uint type : TYPE;
    float4 color : COLOR;
};

float4 main(ParticleVisuals input) : SV_TARGET
{
	return input.color;
}