#include "Basic3D.hlsli"

struct TransformationMatrix
{
    float4x4 wvp;
    float4x4 world;
};
struct WaveBuffer
{
    float time;
    float frequency;
    float amplitude;
};

ConstantBuffer<TransformationMatrix> gTransformationMatrix : register(b0);
ConstantBuffer<WaveBuffer> gWaveBuffer : register(b1);

struct Input
{
    float4 position : POSITION0;
    float2 uv : TEXCOORD0;
    float3 normal : NORMAL0;
};


VertexOutput main(Input input)
{
    VertexOutput output;
    float3 worldPos = mul(input.position, gTransformationMatrix.world).xyz;
    output.uv = input.uv;
    output.normal = normalize(mul(input.normal, (float3x3) gTransformationMatrix.world));
    
    float3 wave = sin(input.position * gWaveBuffer.frequency + gWaveBuffer.time).xyz * gWaveBuffer.amplitude;
    //wave *= input.uv.x;
    worldPos +=  wave * output.normal;
    output.position = mul(float4(worldPos, 1.0f), gTransformationMatrix.wvp);
    //output.position = mul(input.position, gTransformationMatrix.wvp);
    return output;
}