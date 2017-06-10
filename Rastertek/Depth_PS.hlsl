struct PixelInputType
{
    float4 position : SV_Position;
    float4 fixedPointDepth : TEXTURE0;
    float4 linearDepth : TEXCOORD1;
};

float2 ComputeMoments(float depth)
{
    float moment1 = depth;
    float moment2 = depth * depth;

    //Adjusting
    float dx = ddx(depth);
    float dy = ddy(depth);

    moment2 += 0.25f * (dx * dx + dy * dy);

    return float2(moment1, moment2);
}

float4 main(PixelInputType input) : SV_TARGET
{
    float depth = 0.0f;
    float fixedPointShadows = 0.0f;

    input.linearDepth = input.linearDepth / input.linearDepth.w;
    depth = length(input.linearDepth);

    // Get the depth value of the pixel by dividing the Z pixel depth by the homogeneous W coordinate.
    fixedPointShadows = input.fixedPointDepth.z / input.fixedPointDepth.w;

    return float4(ComputeMoments(depth), fixedPointShadows, 1.0f);

}