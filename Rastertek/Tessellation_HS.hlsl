// Input control point
struct VS_CONTROL_POINT_OUTPUT
{
    float4 position : SV_POSITION;
    float4 worldPos : POSITION0;
    float4 color : COLOR;
    float4 normal : NORMAL;
    float4 lightViewPos : TEXCOORD0;
    float4 lightViewPosVSM : TEXCOORD1;
};

// Output control point
struct HS_CONTROL_POINT_OUTPUT
{
    float4 position : SV_POSITION;
    float4 worldPos : POSITION0;
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

#define NUM_CONTROL_POINTS 3

// Patch Constant Function
HS_CONSTANT_DATA_OUTPUT CalcHSPatchConstants(
	InputPatch<VS_CONTROL_POINT_OUTPUT, NUM_CONTROL_POINTS> ip,
	uint PatchID : SV_PrimitiveID)
{
	HS_CONSTANT_DATA_OUTPUT Output;

	// Insert code to compute Output here
	Output.EdgeTessFactor[0] = 
		Output.EdgeTessFactor[1] = 
		Output.EdgeTessFactor[2] = 
		Output.InsideTessFactor = 4.0f; // e.g. could calculate dynamic tessellation factors instead

	return Output;
}

[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("CalcHSPatchConstants")]
HS_CONTROL_POINT_OUTPUT main( 
	InputPatch<VS_CONTROL_POINT_OUTPUT, NUM_CONTROL_POINTS> ip, 
	uint i : SV_OutputControlPointID,
	uint PatchID : SV_PrimitiveID )
{
	HS_CONTROL_POINT_OUTPUT Output;

	// Insert code to compute Output here
    Output.position = ip[i].position;
    Output.worldPos = ip[i].worldPos;
    Output.color = ip[i].color;
    Output.normal = ip[i].normal;
    Output.lightViewPos = ip[i].lightViewPos;
    Output.lightViewPosVSM = ip[i].lightViewPosVSM;

	return Output;
}
