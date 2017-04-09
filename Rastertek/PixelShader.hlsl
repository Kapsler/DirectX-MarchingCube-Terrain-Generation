Texture2D shaderTextures[2] : register(t0);
Texture2D depthMapTexture : register (t2);
Texture2D depthMapTexture2 : register (t3);

SamplerState SampleTypeWrap;
SamplerState SampleTypeClamp;

struct PixelInputType
{
    float4 position : SV_Position;
    float4 color : COLOR;
};

float4 main(PixelInputType input) : SV_TARGET
{
	return input.color;
}