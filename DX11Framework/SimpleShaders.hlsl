#define NUM_LIGHTS 8

// No enums HLSL hates me :<
#define DIRECTIONAL 0
#define POINT 1
#define SPOT 2

cbuffer MatrixBuffer : register(b0)
{
    float4x4 Projection;
    float4x4 View;
    float4x4 World;
}

cbuffer LightVertexBuffer : register(b1)
{
    float4 LightPosition[NUM_LIGHTS];
}

struct LightData
{
    float4 DiffuseColour;
    float4 AmbientColour;
    float4 SpecularColour;
    
    float2 padding;
    int LightType;
    float SpecularPower;
    
    float3 LightDirection;
    float FalloffMultiplier;
};

cbuffer LightPixelBuffer : register(b2)
{
    LightData data[NUM_LIGHTS];
    
    float3 CameraPos;
    int NumActiveLights;
}

Texture2D diffuseTex : register(t0);
Texture2D normalMap : register(t1);
Texture2D roughnessMap : register(t2);
Texture2D aoMap : register(t3);

SamplerState bilinearSampler : register(s0);

float3 CalculateDirectionalLight(float lightIntensity, float3 lightDir, float3 viewDir, float3 normal, float2 texCoord, int lightIndex);
float3 CalculatePointLight(float lightIntensity, float3 lightDir, float3 lightVector, float3 viewDir, float3 normal, float2 texCoord, int index);

struct VS_Out
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD0;
    float3 normal : NORMAL;
    float3 viewDirection : TEXCOORD1;
    float3 lightVector[NUM_LIGHTS] : TEXCOORD2; // Used in point light.
};

VS_Out VS_main(float3 Position : POSITION, float2 Texcoord : TEXCOORD, float3 Normal : NORMAL)
{
    VS_Out output = (VS_Out) 0;

    output.position = mul(float4(Position, 1), World);
    output.position = mul(output.position, View);
    output.position = mul(output.position, Projection);
    
    output.texCoord = Texcoord;
    
    output.normal = mul(Normal, (float3x3) World);
    output.normal = normalize(output.normal);
    
    float4 worldPosition = mul(float4(Position, 1), World);
    
    output.viewDirection = CameraPos.xyz - worldPosition.xyz;
    output.viewDirection = normalize(output.viewDirection);
    
    for (int i = 0; i < NumActiveLights; i++)
    {
        switch (data[i].LightType)
        {
            case DIRECTIONAL:
                float3 lightPos = float3(LightPosition[i].rgb);
                output.lightVector[i] = LightPosition[i].rgb;
                break;
            
            case POINT:
                output.lightVector[i] = LightPosition[i].xyz - worldPosition.xyz;
                break;
            
            default:
                output.lightVector[i] = float3(0.0f, 0.0f, 0.0f);
                break;
        }
    }
    
    return output;
}

float4 PS_main(VS_Out input) : SV_TARGET
{
    float3 finalColour = float3(0, 0, 0);
    
    float4 texel = diffuseTex.Sample(bilinearSampler, input.texCoord);
    float ao = aoMap.Sample(bilinearSampler, input.texCoord).r;
    
    float3 normal = normalize(input.normal);
    float3 viewDir = normalize(input.viewDirection);
    
    // Normal maps need to be sampled otherwise the compiler tries to remove them and misalign texture units.
    // I didn't have time :<<<
    float4 normalValue = normalMap.Sample(bilinearSampler, input.texCoord);
    
    for (int i = 0; i < NumActiveLights; i++)
    {   
        switch (data[i].LightType)
        {
            case DIRECTIONAL:
            {
                    float3 lightDir = normalize(input.lightVector[i]);
                    float lightIntensity = saturate(dot(normal, lightDir));
                    finalColour += CalculateDirectionalLight(lightIntensity, lightDir, viewDir, normal, input.texCoord, i);
                    break;
                }
            
            case POINT:
            {
                    float3 lightDir = normalize(input.lightVector[i]);
                    float lightIntensity = saturate(dot(normal, lightDir));
                    finalColour += CalculatePointLight(lightIntensity, lightDir, input.lightVector[i], viewDir, normal, input.texCoord, i);
                    break;
                }
            
        }
    } 
 
    return float4(saturate(texel.rgb * finalColour * ao), normalValue.a);
}

float3 CalculatePointLight(float lightIntensity, float3 lightDir, float3 lightVector, float3 viewDir, float3 normal, float2 texCoord, int index)
{
    float3 diffuse = float3(0, 0, 0);
    float3 specular = float3(0.0f, 0.0f, 0.0f);
    float attenuation = 0.0f;
    
    float3 ambient = data[index].AmbientColour;
    
    // Only calculate diffuse and specular if nessecary.
    if (lightIntensity > 0.0f)
    {
        diffuse = data[index].DiffuseColour * lightIntensity;
        attenuation = saturate(1.0f - pow(length(lightVector) * data[index].FalloffMultiplier, 2.0f));
        
        // Calculate half vector for Blinn-Phong.
        float3 halfwayVector = normalize(lightDir + viewDir);

        specular = pow(saturate(dot(normal, halfwayVector)), data[index].SpecularPower);
        specular *= data[index].SpecularColour * roughnessMap.Sample(bilinearSampler, texCoord);
    }
    
    return saturate(float3(ambient + diffuse + specular) * attenuation);
}

float3 CalculateDirectionalLight(float lightIntensity, float3 lightDir, float3 viewDir, float3 normal, float2 texCoord, int lightIndex)
{
    float3 normalizedLightDir = normalize(-lightDir);
    
    float3 diffuse = float3(0.0f, 0.0f, 0.0f);
    float3 specular = float3(0.0f, 0.0f, 0.0f);
    
    float3 ambient = data[lightIndex].AmbientColour;
    
    // Only calculate diffuse and specular if nessecary.
    if (lightIntensity > 0.0f)
    {
        diffuse = data[lightIndex].DiffuseColour * lightIntensity;
        
        // Calculate half vector for Blinn-Phong.
        float3 halfwayVector = normalize(lightDir + viewDir);

        specular = pow(saturate(dot(normal, halfwayVector)), data[lightIndex].SpecularPower);
        specular *= data[lightIndex].SpecularColour * roughnessMap.Sample(bilinearSampler, texCoord);
    }
    
    return (ambient + diffuse + specular);
}