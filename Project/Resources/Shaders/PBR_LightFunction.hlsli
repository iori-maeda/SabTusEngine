#pragma onece
#include "LightStatus.hlsli"

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
    return F0 + (1.0f + F0) * pow(saturate(1.0f - HdotV), 5.0f);
}

#define kPI 3.1415926535f


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
    return float4((diffuse + specular) * kLightColor.rgb * kNdotL, surface.albedo.a);

    //float3 diffuseColor = surface.albedo.rgb * (1.0f - surface.metallic);
                    
    //float3 specColor = lerp(float3(0.04f, 0.04f, 0.04f), surface.albedo.rgb, surface.metallic);
                    
    //// 移行のため近似値による実装
    //float normalization = (n + 8.0f) / (8.0f * 3.1415926535f);
    //float roughnessIntensity = pow(saturate(kNdotH), n) * normalization;
    //const float4 kLightDiffuse = float4(diffuseColor, 1.0f) * kLightColor * kNdotL;
    //const float4 kLightSpecular = float4(specColor, 1.0f) * kLightColor * roughnessIntensity;
                
    
    //return kLightDiffuse + kLightSpecular;
}