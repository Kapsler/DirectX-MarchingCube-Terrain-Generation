struct GSInput
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
};

struct GSOuput
{
    float4 position : SV_POSITION;
    float4 worldPos : POSITION;
    float4 color : COLOR;
    float4 normal : NORMAL;
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
float4 getWorldPos(float4 position)
{
    position = mul(position, worldMatrix);
    return position;
}

float4 getProjectionPosition(float4 position)
{
    position = mul(position, viewMatrix);
    position = mul(position, projectionMatrix);
    return position;
}

float3 vertexInterpolation(float isoLevel, float3 v0, float l0, float3 v1, float l1)
{
    float lerper = (isoLevel - l0) / (l1 - l0);
    return lerp(v0, v1, lerper);
}

float3 calculateNormal(float3 p)
{
    p = (p + 1.0f) / 2.0f;

    float3 grad;
    grad.x = densityTex.SampleLevel(samplerPoint, p + float3(dataStep.x, 0, 0), 0) -
             densityTex.SampleLevel(samplerPoint, p - float3(dataStep.x, 0, 0), 0);
    grad.y = densityTex.SampleLevel(samplerPoint, p + float3(0, dataStep.y, 0), 0) -
             densityTex.SampleLevel(samplerPoint, p - float3(0, dataStep.y, 0), 0);
    grad.z = densityTex.SampleLevel(samplerPoint, p + float3(0, 0, dataStep.z), 0) -
             densityTex.SampleLevel(samplerPoint, p - float3(0, 0, dataStep.z), 0);

    return -normalize(grad);
}

/*
Gets a value from the look-up table
*/
int checkTriangleLUT(int i, int j)
{
    if (i >= 256 || j >= 16)
    {
        return -1;
    }

    return tritableTex.Load(int3(j, i, 0));
}

[maxvertexcount(18)]
void main(
	point GSInput input[1], 
	inout TriangleStream<GSOuput> output
)
{
    float isolevel = 0.00f;

    float4 position = input[0].position;
    float4 color = input[0].color;

    float3 cubePoses[8];
    float cubeVals[8];
    for (int i = 0; i < 8; i++)
    {
        cubePoses[i] = position.xyz + decal[i].xyz;
        cubeVals[i] = densityTex.SampleLevel(samplerPoint, (cubePoses[i] + 1.0f) / 2.0f, 0);
    }
    
    int cubeindex = 0;
    cubeindex = int(cubeVals[0] < isolevel);
    cubeindex += int(cubeVals[1] < isolevel) << 1;
    cubeindex += int(cubeVals[2] < isolevel) << 2;
    cubeindex += int(cubeVals[3] < isolevel) << 3;
    cubeindex += int(cubeVals[4] < isolevel) << 4;
    cubeindex += int(cubeVals[5] < isolevel) << 5;
    cubeindex += int(cubeVals[6] < isolevel) << 6;
    cubeindex += int(cubeVals[7] < isolevel) << 7;

    if (cubeindex != 0 && cubeindex != 255)
    {
    
        float3 vertlist[12];

		//Find the vertices where the surface intersects the cube
        vertlist[0] = vertexInterpolation(isolevel, cubePoses[0], cubeVals[0], cubePoses[1], cubeVals[1]);
        vertlist[1] = vertexInterpolation(isolevel, cubePoses[1], cubeVals[1], cubePoses[2], cubeVals[2]);
        vertlist[2] = vertexInterpolation(isolevel, cubePoses[2], cubeVals[2], cubePoses[3], cubeVals[3]);
        vertlist[3] = vertexInterpolation(isolevel, cubePoses[3], cubeVals[3], cubePoses[0], cubeVals[0]);
        vertlist[4] = vertexInterpolation(isolevel, cubePoses[4], cubeVals[4], cubePoses[5], cubeVals[5]);
        vertlist[5] = vertexInterpolation(isolevel, cubePoses[5], cubeVals[5], cubePoses[6], cubeVals[6]);
        vertlist[6] = vertexInterpolation(isolevel, cubePoses[6], cubeVals[6], cubePoses[7], cubeVals[7]);
        vertlist[7] = vertexInterpolation(isolevel, cubePoses[7], cubeVals[7], cubePoses[4], cubeVals[4]);
        vertlist[8] = vertexInterpolation(isolevel, cubePoses[0], cubeVals[0], cubePoses[4], cubeVals[4]);
        vertlist[9] = vertexInterpolation(isolevel, cubePoses[1], cubeVals[1], cubePoses[5], cubeVals[5]);
        vertlist[10] = vertexInterpolation(isolevel, cubePoses[2], cubeVals[2], cubePoses[6], cubeVals[6]);
        vertlist[11] = vertexInterpolation(isolevel, cubePoses[3], cubeVals[3], cubePoses[7], cubeVals[7]);

        GSOuput v0;
        GSOuput v1;
        GSOuput v2;

        for (int i = 0; checkTriangleLUT(cubeindex, i) != -1; i += 3)
        {
            //Set Color
            v0.color = float4(1.0f, 1.0f, 1.0f, 1.0f);
            v1.color = float4(1.0f, 1.0f, 1.0f, 1.0f);
            v2.color = float4(1.0f, 1.0f, 1.0f, 1.0f);

            //Get Case from LUT
            v0.position = float4(vertlist[checkTriangleLUT(cubeindex, i + 0)], 1);
            v1.position = float4(vertlist[checkTriangleLUT(cubeindex, i + 1)], 1);
            v2.position = float4(vertlist[checkTriangleLUT(cubeindex, i + 2)], 1);
            
            //Get World Pos
            v0.worldPos = getWorldPos(v0.position);
            v1.worldPos = getWorldPos(v1.position);
            v2.worldPos = getWorldPos(v2.position);

            //Calculate Normals
            v0.normal.xyz = calculateNormal(v0.worldPos.xyz);
            v1.normal.xyz = calculateNormal(v1.worldPos.xyz);
            v2.normal.xyz = calculateNormal(v2.worldPos.xyz);
            v0.normal.w = 1.0f;
            v1.normal.w = 1.0f;
            v2.normal.w = 1.0f;

            //Output Vertices
            output.Append(v0);
            output.Append(v1);
            output.Append(v2);
            output.RestartStrip();
        }
    }   

}