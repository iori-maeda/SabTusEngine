#pragma once
enum LightType
{
    DIRECTIONAL = 0,
    POINT = 1,
    SPOT = 2
};

struct LightStatus
{
    float4 color;
    float3 direction;
    float intensity;
    float3 position;
    float range;
    float decay;
    float cosFallOffStart;
    float cosAngle;
    uint type;
};