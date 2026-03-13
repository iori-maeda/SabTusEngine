#pragma once
struct MeshMaterial
{
    float4 Ka;
    float4 Kd;
    float4 Ks;
    float shininess;
    uint32_t isUseArmTex;
    uint32_t ambientOclusionChannel;
    uint32_t roughnessChannel;
    uint32_t metallicChannel;
};
struct Essential
{
    uint numLights;
};

struct ObjectMaterial
{
    float4 color;
    uint32_t enableLighting;
    uint32_t useNormal;
    uint32_t useRoughness;
};

struct Camera
{
    float3 worldPosition;
    float4x4 viewMatrix;
};

struct FogStatus
{
    float density;
    float power;
    float thresholdStart;
    float thresholdEnd;
};