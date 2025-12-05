cbuffer MatrixBuffer : register(b0)
{
	float4x4 Projection;
	float4x4 View;
	float4x4 World;
}

cbuffer BlurBuffer : register(b1)
{
    float2 windowDimensions;
    int kernelSize = 1;
    float sigma = 1.0f;
    float samplePosMult = 1.0f;
    float3 padding;
}

//cbuffer GaussianBuffer : register(b1)
//{
//}

struct VS_OUT
{
	float4 position : SV_POSITION;
	float2 texCoord : TEXCOORD;
};

Texture2D screenTex : register(t0);
SamplerState bilinearSampler : register(s0);

VS_OUT VS_main(float3 Position : POSITION, float2 Texcoord : TEXCOORD, float3 Normal : NORMAL)
{
	VS_OUT output = (VS_OUT) 0;

	output.position = mul(float4(Position, 1), World);
	output.position = mul(output.position, View);
	output.position = mul(output.position, Projection);
    
	output.texCoord = Texcoord;
    
    return output;
}

float gaussianWeight(float x, float sigma)
{
    return exp(-pow(x, 2) / (2.0 * pow(sigma, 2)));
}

float4 PS_main(VS_OUT input) : SV_TARGET
{
	float4 finalColour;
    float4 boxBlurColour;
    
    float weightSum = 0.0f;

    
    // Could maybe have box/ gaussian be toggleable.
    //float boxBlurDivisor = pow(2.0f * kernelSize + 1.0, 2);
	
    float2 texelSize = 1.0f / windowDimensions;
    
    for (int y = -kernelSize; y <= kernelSize; ++y)
    {
        for (int x = -kernelSize; x <= kernelSize; ++x)
        {
            float2 offset = float2(x, y) * samplePosMult * texelSize;
            boxBlurColour += screenTex.Sample(bilinearSampler, input.texCoord + offset);
            
            float weight = gaussianWeight(x, sigma);
            weightSum += weight;
        }
    }
	
    return finalColour = boxBlurColour / weightSum;
}