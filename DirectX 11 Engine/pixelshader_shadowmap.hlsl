cbuffer lightViewProjectionBuffer : register(b0)
{
    float4x4 world;
    float4x4 view;
    float4x4 projection;
};

cbuffer worldViewProjectionBuffer : register(b1)
{
    float4x4 world;
    float4x4 view;
    float4x4 projection;
};

struct PixelShaderInput
{
    float4 pos : SV_POSITION;
    float4 lpos : TEXCOORD0; //vertex with regard to light view
    float3 normal : NORMAL;
};
 
//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PixelShaderInput VS(VS_INPUT input)
{
    PixelShaderInput output;
    float4 pos = float4(input.pos, 1.0f);
    // Transform the vertex position into projected space.
    pos = mul(pos, world);
    pos = mul(pos, view);
    pos = mul(pos, projection);
    output.pos = pos;

    output.pos = mul(input.pos, mul(world, viewProj));
    output.normal = input.normal;
 
    //store worldspace projected to light clip space with
    //a texcoord semantic to be interpolated across the surface
    output.lpos = mul(input.pos, mul(world, lightViewProj));
 
    return output;
}