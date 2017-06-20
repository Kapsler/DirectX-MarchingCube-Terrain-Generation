struct DS_OUTPUT
{
    float4 position : SV_POSITION;
    float4 worldPos : POSITION;
    float4 color : COLOR;
    float4 normal : NORMAL;
    float4 lightViewPos : TEXCOORD0;
    float4 lightViewPosVSM : TEXCOORD1;
};

// Output control point
struct HS_CONTROL_POINT_OUTPUT
{
    float4 position : SV_POSITION;
    float4 worldPos : POSITION;
    float4 color : COLOR;
    float4 normal : NORMAL;
    float4 lightViewPos : TEXCOORD0;
    float4 lightViewPosVSM : TEXCOORD1;
};

// Output patch constant data.
struct HS_CONSTANT_DATA_OUTPUT
{
	float EdgeTessFactor[3]			: SV_TessFactor; // e.g. would be [4] for a quad domain
	float InsideTessFactor			: SV_InsideTessFactor; // e.g. would be Inside[2] for a quad domain
	// TODO: change/add other stuff
};

cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

cbuffer LightMatrixBuffer : register(b1)
{
    matrix lightViewMatrix;
    matrix lightProjectionMatrix;
};

#define NUM_CONTROL_POINTS 3

[domain("tri")]
DS_OUTPUT main(
	HS_CONSTANT_DATA_OUTPUT input,
	float3 domain : SV_DomainLocation,
	const OutputPatch<HS_CONTROL_POINT_OUTPUT, NUM_CONTROL_POINTS> patch)
{
	DS_OUTPUT Output;

	Output.position = float4(
		patch[0].position.xyz  *domain.x
        + patch[1].position.xyz * domain.y
        + patch[2].position.xyz * domain.z, 
        1);
    
    Output.worldPos = float4(
		patch[0].worldPos.xyz * domain.x
        + patch[1].worldPos.xyz * domain.y
        + patch[2].worldPos.xyz * domain.z,
        1);

    Output.color = float4(
		patch[0].color.xyz * domain.x
        + patch[1].color.xyz * domain.y
        + patch[2].color.xyz * domain.z,
        1);

    Output.normal = float4(
		patch[0].normal.xyz * domain.x
        + patch[1].normal.xyz * domain.y
        + patch[2].normal.xyz * domain.z,
        1);
    
    Output.lightViewPos = float4(
		patch[0].lightViewPos.xyz * domain.x
        + patch[1].lightViewPos.xyz * domain.y
        + patch[2].lightViewPos.xyz * domain.z,
        1);

    Output.lightViewPosVSM = float4(
		patch[0].lightViewPosVSM.xyz * domain.x
        + patch[1].lightViewPosVSM.xyz * domain.y
        + patch[2].lightViewPosVSM.xyz * domain.z,
        1);

    Output.lightViewPos = mul(Output.lightViewPos, lightProjectionMatrix);
    Output.position = mul(Output.position, projectionMatrix);

	return Output;
}
