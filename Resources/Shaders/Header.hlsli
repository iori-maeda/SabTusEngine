#define NUM_CONTROL_POINTS 3

struct TransformationMatrix
{
    float4x4 wvp;
    float4x4 world;
};

ConstantBuffer<TransformationMatrix> gTransformationMatrix : register(b99);

struct VertexOutput
{
    float3 worldPos : WORLDPOS;
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
    float3 normal : NORMAL0;
};

struct HS_CONTROL_POINT_OUTPUT
{
    float3 vPosition : WORLDPOS;
    float2 uv : TEXCOORD0;
    float3 normal : NORMAL0;
};

struct HS_CONSTANT_DATA_OUTPUT
{
    float EdgeTessFactor[3] : SV_TessFactor;
    float InsideTessFactor : SV_InsideTessFactor;
};

struct DS_OUTPUT
{
    float4 vPosition : SV_POSITION;
    float2 uv : TEXCOORD0;
    float3 normal : NORMAL0;
};