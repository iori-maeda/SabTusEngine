#pragma onece
#include "LightStatus.hlsli"

#define kPI 3.1415926535f

struct PBR_SurfaceStatus
{
    float3 worldPos;
    float3 normal;
    float4 albedo;
    float roughness;
    float metallic;
};

// F0 VerticalInputVectorReflection
float3 SchickFrenel(float3 F0, float HdotV)
{
    return F0 + (1.0f - F0) * pow(saturate(1.0f - HdotV), 5.0f);
}

float3 CaluclateBRDF(PBR_SurfaceStatus surface, float3 V, float3 L, float3 H)
{
    const float3 N = normalize(surface.normal);
    
    // Normal dot LightDir
    const float kNdotL = saturate(dot(N, L));
    // Normal dot EyeDir
    const float kNdotV = saturate(dot(N, V));
    // Normal dot HalfVec
    const float kNdotH = saturate(dot(N, H));
    // HalfVec dot EyeDir
    const float kHdotV = saturate(dot(H, V));
    
    const float3 unMetallic = float3(0.04f, 0.04f, 0.04f);
    
    // 1. Frenel (F)
    // 非金属0.04 金属はF0をアルベドカラーに
    float3 F0 = lerp(unMetallic, surface.albedo.rgb, surface.metallic);
    float3 F = SchickFrenel(F0, kHdotV);
    
    // 2. Normal Distribution Func (D)
    float alpha = surface.roughness * surface.roughness;
    float n = 2.0f / (alpha + 0.0001f) - 2.0f;
    float D = pow(kNdotH, max(n, 0.0f)) * ((n + 2.0f) / (2.0f * kPI));
    
    // 3. Supecular BRDF (簡略化)
    float3 specular = (F * D) / 4.0f;
    
    // 4. Diffuse
    // 金属であるほど拡散しない
    float3 kD = (1.0f - F) * (1.0f - surface.metallic);
    float3 diffuse = kD * surface.albedo.rgb / kPI;
    
    // 5. result
    return diffuse + specular;
}



float4 CaluclateDirectionalLightPBR(LightStatus light, float3 cameraPosition, PBR_SurfaceStatus surface)
{
    // Base Light Color
    const float4 kLightColor = light.color * light.intensity;
    
    // EyeDirection
    const float3 V = normalize(cameraPosition - surface.worldPos);
    // LightDirection
    const float3 L = -normalize(light.direction);
    // HalfVector
    const float3 H = normalize(L + V);
    // Normal
    const float3 N = normalize(surface.normal);
    // Normal dot LightDir
    const float kNdotL = saturate(dot(N, L));
   
    float3 BRDF = CaluclateBRDF(surface, V, L, H);
    
    // 5. result
    return float4(BRDF * kLightColor.rgb * kNdotL, surface.albedo.a);
}

float4 CaluclatePointLightColorPBR(const LightStatus light, float3 cameraPosition, PBR_SurfaceStatus surface)
{
    const float4 kLightColor = light.color * light.intensity;
    
    // 入射ベクトルがないので相対位置から算出
    const float3 kToLight = light.position - surface.worldPos;
    const float kToLightLength = length(kToLight);
    
    // EyeVector
    const float3 V = normalize(cameraPosition - surface.worldPos);
    const float3 L = normalize(kToLight);
    const float3 H = normalize(L + V);
    const float3 N = normalize(surface.normal);
    
    const float kNdotL = saturate(dot(N, L));
    
    const float3 BRDF = CaluclateBRDF(surface, V, L, H);
    
    // 距離減衰
    const float kRange = max(light.range, 0.0001f);
    const float kAttenuationFactor = pow(saturate(1.0f - kToLightLength / kRange), light.decay);
    
    const float3 kEffectiveLightColor = kLightColor.rgb * kAttenuationFactor;
    
    
    return float4(BRDF * kEffectiveLightColor * kNdotL, surface.albedo.a);
}

float4 CaluclateSpotLightColorPBR(const LightStatus light, float3 cameraPosition, PBR_SurfaceStatus surface)
{
    const float3 kToLight = light.position - surface.worldPos;
    const float3 kLightDirectionNormal = -normalize(light.direction);
    const float kToLightLength = length(kToLight);
     
    const float4 kLightColor = light.color * light.intensity;
    
    const float3 V = normalize(cameraPosition - surface.worldPos);
    const float3 L = normalize(kToLight);
    const float3 H = normalize(V + L);
    const float3 N = normalize(surface.normal);
    
    const float kNdotL = saturate(dot(N, L));
    
    // 距離減衰
    const float kRange = max(light.range, 0.0001f);
    const float kAttenuationFactor = pow(saturate(1.0f - kToLightLength / kRange), light.decay);
    // 範囲減衰
    const float kCosAngle = dot(L, kLightDirectionNormal);
    const float kCosAngleDiff = max(light.cosFallOffStart - light.cosAngle, 0.0001f);
    const float kFalloffFactor = saturate((kCosAngle - light.cosAngle) / kCosAngleDiff);
    
    const float3 kEffectiveLightColor = kLightColor.rgb * kAttenuationFactor * kFalloffFactor;
    
    const float3 BRDF = CaluclateBRDF(surface, V, L, H);
    
    return float4(BRDF * kEffectiveLightColor * kNdotL, surface.albedo.a);
}
