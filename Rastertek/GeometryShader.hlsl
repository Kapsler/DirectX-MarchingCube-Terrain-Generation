struct GSInput
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
};

struct PixelInputType
{
	float4 position : SV_POSITION;
	float4 color : COLOR0;
};

cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};


cbuffer DecalDescription : register(b1)
{
    float4 decal[8];
    float4 dataStep;
}

//Generic sampler
SamplerState samplerPoint : register(s0);
//The texture containing the density values
Texture3D<float> densityTex : register(t0);
//Triangle look up table
Texture2D<int> tritableTex : register(t1);

/*
Multiplies the position by the world, view and projection matrix
*/
float4 getProjectionPos(float4 position)
{
    position = mul(position, worldMatrix);
    position = mul(position, viewMatrix);
    position = mul(position, projectionMatrix);
    return position;
}

[maxvertexcount(3)]
void main(
	point GSInput input[1] : SV_POSITION, 
	inout TriangleStream<PixelInputType> output
)
{
	PixelInputType element;
    float offset = 0.02f;

    float4 position = getProjectionPos(input[0].position);
    float density = densityTex.SampleLevel(samplerPoint, (input[0].position.xyz + 1) / 2.0f, 0);
    float4 color = float4(density, density, 0.1f, 1.0f);
    
    //Triangle Begin
    element.position = position;
    element.position.x -= offset;
    element.position.y -= offset;
    element.color = color;
	output.Append(element);

    element.position = position;
    element.position.y += offset;
    element.color = color;
    output.Append(element);

    element.position = position;
    element.position.x += offset;
    element.position.y -= offset;
    element.color = color;
    output.Append(element);
    //Triangle End

    output.RestartStrip();
	
}