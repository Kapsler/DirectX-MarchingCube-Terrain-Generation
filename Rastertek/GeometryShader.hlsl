struct GSInput
{
	float4 pos : SV_POSITION;
	float4 color : COLOR;
};

struct PixelInputType
{
	float4 position : SV_POSITION;
	float4 color : COLOR0;
};

cbuffer MatrixBuffer
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

//Generic sampler
SamplerState samplerPoint : register(s0);
//The texture containing the density values
Texture3D<float> densityTex : register(t0);

[maxvertexcount(3)]
void main(
	point GSInput input[1] : SV_POSITION, 
	inout TriangleStream<PixelInputType> output
)
{
	PixelInputType element;
    float offset = 0.05f;

    float4 position = input[0].pos;
    position = mul(position, worldMatrix);
    position = mul(position, viewMatrix);
    position = mul(position, projectionMatrix);
    float density = densityTex.SampleLevel(samplerPoint, (input[0].pos.xyz + 1) / 2.0f, 0);
    float4 color = float4(density, 0.1f, 0.1f, 1.0f);
    
    //Triangle Begin
    element.position = position;
    element.position.x -= offset / 2.0f;
    element.position.y -= offset / 2.0f;
    element.color = color;
	output.Append(element);

    element.position = position;
    element.position.y += offset / 2.0f;
    element.color = color;
    output.Append(element);

    element.position = position;
    element.position.x += offset / 2.0f;
    element.position.y -= offset / 2.0f;
    element.color = color;
    output.Append(element);
    //Triangle End

    output.RestartStrip();
	
}