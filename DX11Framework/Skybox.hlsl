cbuffer ConstantBuffer : register(b0)
{
    float4x4 Projection;
    float4x4 View;
    float4x4 World;
    
    float4 DiffuseColour;
    float4 DiffuseMaterial;
    
    float4 AmbientColour;
    float4 AmbientMaterial;
    
    float4 SpecularColour;
    float4 SpecularMaterial;
    
    float3 CameraPos;
    float SpecularPower;
    
    float3 LightDirection;
    uint LightCount;
    
    uint HasTexture;
}

struct SkyboxVS_Out
{
    float4 position : SV_POSITION;
    float3 texCoord : TEXCOORD;
};

TextureCube skyboxTex : register(t0);
SamplerState bilinearSampler : register(s0);

SkyboxVS_Out VS_main(float3 Position : POSITION, float2 Texcoord : TEXCOORD, float3 Normal : NORMAL)
{
    SkyboxVS_Out output = (SkyboxVS_Out) 0;

    output.position = mul(float4(Position, 1), World);
    output.position = mul(output.position, View);
    output.position = mul(output.position, Projection);
    
    output.position = output.position.xyww;
    
    output.texCoord = Position;
    
    return output;
}

float4 PS_main(SkyboxVS_Out input) : SV_TARGET
{
    float4 finalColour;
    
    finalColour = skyboxTex.Sample(bilinearSampler, input.texCoord);
    
    return finalColour;
}