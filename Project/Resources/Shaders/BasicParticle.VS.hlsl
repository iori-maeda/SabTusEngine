#include "BasicParticle.hlsli"

struct ParticleForGPU
{
    float4x4 wvp;
    float4x4 world;
    float4 color;
};

StructuredBuffer<ParticleForGPU> gParticle : register(t0);

struct Input
{
    float4 position : POSITION0;
    float2 uv : TEXCOORD0;
};


VertexOutput main(Input input, uint insatanceId : SV_InstanceID)
{
    VertexOutput output;
    output.position = mul(input.position, gParticle[insatanceId].wvp);
    output.uv = input.uv;
    output.color = gParticle[insatanceId].color;
    return output;
}