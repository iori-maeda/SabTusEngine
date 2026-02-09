#include "BasicParticle.hlsli"

struct Transform
{
    float3 scale;
    float3 rotation;
    float3 potision;
};

struct ParticleForGPU
{
    Transform transform;
    float4 color;
    int useBillBoard;
};

struct CameraData
{
    float3 position;
    float4x4 viewMat;
    float4x4 projMat;
};

StructuredBuffer<ParticleForGPU> gParticle : register(t0);
ConstantBuffer<CameraData> gCameraData : register(b0);

struct Input
{
    float4 position : POSITION0;
    float2 uv : TEXCOORD0;
};

float4x4 TranslateMat(float3 position)
{
    float4x4 translateMat =
    {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        position.x, position.y, position.z, 1.0f
    };
    return translateMat;
}

float4x4 ScaleMatrix(float3 scale)
{
    float4x4 scaleMat =
    {
        scale.x, 0.0f, 0.0f, 0.0f,
        0.0f, scale.y, 0.0f, 0.0f,
        0.0f, 0.0f, scale.y, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
    return scaleMat;
}

float4x4 RotateMatrix(float3 rotation)
{
    // X Rotate 
    float sinX = sin(rotation.x);
    float cosX = cos(rotation.x);
    float4x4 rotateX =
    {
        1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, cosX, sinX, 0.0f,
	0.0f, -sinX, cosX, 0.0f,
	0.0f, 0.0f, 0.0f, 1.0f,
    };

    // Y Rotate
    float sinY = sin(rotation.y);
    float cosY = cos(rotation.y);
    float4x4 rotateY =
    {
        cosY, 0.0f, -sinY, 0.0f,
	 0.0f, 1.0f, 0.0f, 0.0f,
	  sinY, 0.0f, cosY, 0.0f,
	 0.0f, 0.0f, 0.0f, 1.0f,
    };

    // Z Rotate
    float sinZ = sin(rotation.z);
    float cosZ = cos(rotation.z);
    float4x4 rotateZ =
    {
        cosZ, sinZ, 0.0f, 0.0f,
	 -sinZ, cosZ, 0.0f, 0.0f,
	 0.0f, 0.0f, 1.0f, 0.0f,
	 0.0f, 0.0f, 0.0f, 1.0f
    };


    return rotateZ * rotateX * rotateY;
}


VertexOutput main(Input input, uint insatanceId : SV_InstanceID)
{
    VertexOutput output;
    
    float4x4 worldMat = 
    ScaleMatrix(gParticle[insatanceId].transform.scale)
    * RotateMatrix(gParticle[insatanceId].transform.rotation)
    * TranslateMat(gParticle[insatanceId].transform.potision);
    
    float4x4 WVP = worldMat * gCameraData.viewMat * gCameraData.projMat;
    
    output.position = mul(input.position, WVP);
    output.uv = input.uv;
    output.color = gParticle[insatanceId].color;
    return output;
}