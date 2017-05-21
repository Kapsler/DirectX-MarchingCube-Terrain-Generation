struct ParticleVisuals
{
    float4 position : SV_POSITION;
    uint type : TYPE;
    float age : AGE;
    float4 color : COLOR;
};

float4 main(ParticleVisuals input) : SV_TARGET
{
	return input.color;
}