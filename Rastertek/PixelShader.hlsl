Texture2D triplanarTexX[2] : register(t0);
Texture2D triplanarTexY[2] : register(t2);
Texture2D triplanarTexZ[2] : register(t4);

SamplerState SampleTypeWrap;
SamplerState SampleTypeClamp;

struct PixelInputType
{
    float4 position : SV_POSITION;
    float4 worldPos : POSITION;
    float4 color : COLOR0;
    float4 normal : NORMAL;
};

cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

cbuffer Eye : register(b2)
{
    float3 eyePos;
    float padding1;
    float3 eyeLook;
    float padding2;
    float3 eyeUp;
    float padding3;
};

float2 displacementMapping(float2 uv, Texture2D HeightMap, float4 viewDir)
{ 
    //Displacement
    
    float2 dUV = -viewDir.xy * 0.08; //~displm’t depth
    float prev_hits = 0;
    float hit_h = 0; // THE OUTPUT
    float h = 1.0f;
    for (int it = 0; it < 10; it++)
    {
        h -= 0.1f;
        uv += dUV;
        float h_tex = HeightMap.SampleLevel(SampleTypeWrap, uv, 0).x;
        float is_first_hit = saturate((h_tex - h - prev_hits) * 4999999);
        hit_h += is_first_hit * h;
        prev_hits += is_first_hit;
    }

    return uv;
    //Displacement - End
}

float4 main(PixelInputType input) : SV_TARGET
{
    float tex_scale = 7.0f;
    float4 ambient = float4(0.1f, 0.1f, 0.1f, 1.0f);
    ////float4 ambient = float4(0.0f, 0.0f, 0.0f, 1.0f);
    float4 diffuse = float4(1.0f, 1.0f, 1.0f, 1.0f);
    float4 lightPos = (100.0f, 200.0f, 500.0f, 1.0f);
    float4 finalColor = ambient;

    //Get Viewing Direction
    float4 viewDir = float4(eyePos, 1.0f);
    viewDir = normalize(mul(viewDir, worldMatrix) - input.worldPos);

    // Determine the blend weights for the 3 planar projections.  
    // N_orig is the vertex-interpolated normal vector.  
    float3 blend_weights = abs(input.normal.xyz); // Tighten up the blending zone:  
    blend_weights = (blend_weights - 0.2) * 7;
    blend_weights = max(blend_weights, 0); // Force weights to sum to 1.0 (very important!)  
    blend_weights /= (blend_weights.x + blend_weights.y + blend_weights.z).xxx;
    
    // Now determine a color value and bump vector for each of the 3  
    // projections, blend them, and store blended results in these two  
    // vectors:  
    float4 blended_color; // .w hold spec value  
    float3 blended_bump_vec;
    float uv;
    {  
        // Compute the UV coords for each of the 3 planar projections.  
        // tex_scale (default ~ 1.0) determines how big the textures appear.  
        float2 coord1 = input.worldPos.yz * tex_scale;
        float2 coord2 = input.worldPos.zx * tex_scale;
        float2 coord3 = input.worldPos.xy * tex_scale;

        // This is where you would apply conditional displacement mapping.  
        if (blend_weights.x > 0)
        { 
            coord1 = displacementMapping(coord1, triplanarTexX[1], viewDir);
        }

        if (blend_weights.y > 0)
        {
            coord2 = displacementMapping(coord2, triplanarTexY[1], viewDir);
        }

        if (blend_weights.y > 0)
        {
            coord3 = displacementMapping(coord3, triplanarTexZ[1], viewDir);
        } 

        // Sample color maps for each projection, at those UV coords.  
        float4 col1 = triplanarTexX[0].Sample(SampleTypeWrap, coord1);
        float4 col2 = triplanarTexY[0].Sample(SampleTypeWrap, coord2);
        float4 col3 = triplanarTexZ[0].Sample(SampleTypeWrap, coord3);

        // Sample bump maps too, and generate bump vectors.  
        // (Note: this uses an oversimplified tangent basis.)  
        float2 bumpFetch1 = triplanarTexX[1].Sample(SampleTypeWrap, coord1).xy - 0.5;
        float2 bumpFetch2 = triplanarTexY[1].Sample(SampleTypeWrap, coord2).xy - 0.5;
        float2 bumpFetch3 = triplanarTexZ[1].Sample(SampleTypeWrap, coord3).xy - 0.5;

        float3 bump1 = float3(0, bumpFetch1.x, bumpFetch1.y);
        float3 bump2 = float3(bumpFetch2.y, 0, bumpFetch2.x);
        float3 bump3 = float3(bumpFetch3.x, bumpFetch3.y, 0);

         // Finally, blend the results of the 3 planar projections.  
        blended_color = col1.xyzw * blend_weights.xxxx +
                col2.xyzw * blend_weights.yyyy +
                col3.xyzw * blend_weights.zzzz;

        blended_bump_vec = bump1.xyz * blend_weights.xxx +
                   bump2.xyz * blend_weights.yyy +
                   bump3.xyz * blend_weights.zzz;
    }



    // Apply bump vector to vertex-interpolated normal vector.  
    float4 N_for_lighting;
    N_for_lighting.xyz = normalize(input.normal.xyz + blended_bump_vec);
    N_for_lighting.w = 0.0f;

    //Diffuse Light
    float4 lightDir = -normalize(lightPos - input.position);

    float lightIntensity = saturate(dot(N_for_lighting, lightDir));
    if (lightIntensity > 0.0f)
    {
        finalColor += (diffuse * lightIntensity);
    }
    finalColor = saturate(finalColor);

    finalColor = finalColor * blended_color;

    return finalColor;
}