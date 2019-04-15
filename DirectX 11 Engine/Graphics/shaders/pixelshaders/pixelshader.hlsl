Texture2D shaderTexture : TEXTURE : register(t0);
SamplerState SampleTypeWrap : SAMPLER : register(s0);

cbuffer LightBuffer : register(b0)
{   
    float3 ambientColor;
    float ambientStrength;

    float3 diffuseColor;
    float lightStrength;

    float3 lightPos;
    float lightAttenuation_a;
    
    float lightAttenuation_b;
    float lightAttenuation_c;
}

struct PS_INPUT
{
    float4 inPos : SV_POSITION;
    float2 inTexCoord : TEXCOORD0;
    float3 inNormal : NORMAL;
    float3 inWorldPos : WORLD_POSITION;
};

float4 main(PS_INPUT input) : SV_Target
{
    float3 appliedLight;
    float3 sampleColor;
    float3 ambientLight;
    float3 lightDiffuse;
    float attenuationFactor;
    float distanceToLight;

    //caluclate ambient light
    ambientLight = ambientColor * ambientStrength;
    appliedLight = ambientLight;
    
    //Find light one position
    float3 vectorToLight = normalize(lightPos - input.inWorldPos);
    //calculate radius / attenuation of light one
    float3 LightRadius = max(dot(vectorToLight, input.inNormal), 0);

    //calculate distance and final attenuation of light one
    distanceToLight = distance(lightPos, input.inWorldPos);
    attenuationFactor = 1 / (lightAttenuation_a + lightAttenuation_b * distanceToLight + lightAttenuation_c * pow(distanceToLight, 2));
    LightRadius *= attenuationFactor;

    //calculate diffuse of light one
    lightDiffuse = LightRadius * lightStrength * diffuseColor;

    appliedLight += lightDiffuse;

    // Sample the pixel color from the texture using the sampler at this texture coordinate location.
    sampleColor = shaderTexture.Sample(SampleTypeWrap, input.inTexCoord);

    // Combine the light and sample color.
    float3 finalColor = sampleColor * appliedLight;

    return float4(finalColor, 1.0f);
}
