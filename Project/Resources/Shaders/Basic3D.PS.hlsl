#include "Basic3D.hlsli"
#include "LightFunction.hlsli"

enum LightType
{
    DIRECTIONAL = 0,
    POINT = 1,
    SPOT = 2
};

struct MeshMaterial
{
    float4 Ka;
    float4 Kd;
    float4 Ks;
    float shininess;
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

ConstantBuffer<MeshMaterial> gMeshMaterial : register(b0);
ConstantBuffer<Essential> gEssential : register(b1);
ConstantBuffer<Camera> gCamera : register(b2);
ConstantBuffer<ObjectMaterial> gObjectMaterial : register(b3);
ConstantBuffer<FogStatus> gFogSattus : register(b4);

StructuredBuffer<LightStatus> gLights : register(t0);

Texture2D<float4> gTexture : register(t1);
Texture2D<float4> gNormalTexture : register(t2);
Texture2D<float4> gRoughnessTexture : register(t3);
Texture2D<float4> gMetallicTexture : register(t4);
SamplerState gSampler : register(s0);

struct Output
{
    float4 color : SV_TARGET0;
};

Output main(VertexOutput input)
{
    Output output;
    output.color = float4(0.0f, 0.0f, 0.0f, 1.0f); // Initialize output color

    float4 texColor = gTexture.Sample(gSampler, input.uv);
    float4 roughnessTexColor = gRoughnessTexture.Sample(gSampler, input.uv);
    
    float4 normalTexColor = gNormalTexture.Sample(gSampler, input.uv);
    float3 normal = normalize(input.normal);
    float3 tangent = normalize(input.tangent);
    float3 binormal = normalize(input.binormal);
    float3x3 tangentBinormalMat = float3x3(tangent, binormal, normal);
    
    float3 newNormal = (normalTexColor * 2.0f - 1.0f).rgb;
    
    newNormal = normalize(mul(newNormal, tangentBinormalMat));
    
    
    // ラフネスの取得
    float roughness = gRoughnessTexture.Sample(gSampler, input.uv).r;
    
    //output.color = normalTexColor;
    //return output;
	
    if (texColor.a <= 0.0f)
    {
        discard;
    }
    
    // Base Color
    const float4 kObjectAmbientColor = gMeshMaterial.Ka * texColor * gObjectMaterial.color;
    const float4 kObjectDiffuseColor = gMeshMaterial.Kd * texColor * gObjectMaterial.color;
    const float4 kObjectSpecularColor = gMeshMaterial.Ks * gObjectMaterial.color;
    
    const float4 useAmbientColor = length(gMeshMaterial.Ka) >= length(gMeshMaterial.Kd) ? kObjectAmbientColor * 0.5f : kObjectAmbientColor;
    
    if (gObjectMaterial.enableLighting == 0)
    {
        output.color = kObjectAmbientColor + kObjectDiffuseColor + kObjectSpecularColor;
        output.color.a = texColor.a;
        return output;
    }
    
    // View Direction
    const float3 kToEyeDir = normalize(gCamera.worldPosition - input.worldPosition);
    
    const float3 kNormal = gObjectMaterial.useNormal ? newNormal : normalize(input.normal);
    float4 finaleColor = useAmbientColor;
    
    for (uint i = 0; i < gEssential.numLights; i++)
    {
        // 共通処理
        // 光源色
        const float4 kLightColor = gLights[i].color * gLights[i].intensity;
        
        switch (gLights[i].type)
        {
            case DIRECTIONAL:
                {
                    const float3 kLightDirectionNormal = -normalize(gLights[i].direction);
                    const float kNdotL = saturate(dot(kNormal, kLightDirectionNormal));
                    const float3 kHalfVector = normalize(kLightDirectionNormal + kToEyeDir);
                    const float kNdotH = saturate(dot(kNormal, kHalfVector));
                    
                    const float kSpeclarIntensity = pow(saturate(kNdotH), gMeshMaterial.shininess);
                    const float4 kLightDiffuse = kObjectDiffuseColor * kLightColor * kNdotL;
                    const float4 kLightSpecular = kObjectSpecularColor * kLightColor * kSpeclarIntensity;
                
                    finaleColor += kLightDiffuse + kLightSpecular;
                }
                break;
            
            case POINT:
                {
                    const float3 kToLight = gLights[i].position - input.worldPosition;
                    const float kToLightLength = length(kToLight);
                    if (kToLightLength > gLights[i].range)
                    {
                        continue;
                    }
                
                    const float3 kToLightNormal = normalize(kToLight);
                    const float kNdotL = saturate(dot(kNormal, kToLightNormal));
                    const float3 kHalfVector = normalize(kToLightNormal + kToEyeDir);
                    const float kNdotH = saturate(dot(kNormal, kHalfVector));
                    const float kSpeclarIntensity = pow(saturate(kNdotH), gMeshMaterial.shininess);
                    // 距離減衰
                    const float kRange = max(gLights[i].range, 0.0001f);
                    const float kAttenuationFactor = pow(saturate(1.0f - kToLightLength / kRange), gLights[i].decay);
                  
                    const float4 kLightDiffuse = kObjectDiffuseColor * kLightColor * kNdotL * kAttenuationFactor;
                    const float4 kLightSpecular = kObjectSpecularColor * kLightColor * kSpeclarIntensity * kAttenuationFactor;
               
                    finaleColor += kLightDiffuse + kLightSpecular;
                }
                break;
            
            case SPOT:
                {
                    const float3 kToLight = gLights[i].position - input.worldPosition;
                    const float kToLightLength = length(kToLight);
                    if (kToLightLength > gLights[i].range)
                    {
                        continue;
                    }
                
                    const float3 kLightDirectionNormal = -normalize(gLights[i].direction);
                    const float3 kToLightNormal = normalize(kToLight);
                    const float kNdotL = saturate(dot(kNormal, kToLightNormal));
                    const float3 kHalfVector = normalize(kToLightNormal + kToEyeDir);
                    const float kNdotH = saturate(dot(kNormal, kHalfVector));
                    const float kSpeclarIntensity = pow(saturate(kNdotH), gMeshMaterial.shininess);
                    // 距離減衰
                    const float kRange = max(gLights[i].range, 0.0001f);
                    const float kAttenuationFactor = pow(saturate(1.0f - kToLightLength / kRange), gLights[i].decay);
                    // 範囲減衰
                    const float kCosAngle = dot(kToLightNormal, kLightDirectionNormal);
                    const float kCosAngleDiff = max(gLights[i].cosFallOffStart - gLights[i].cosAngle, 0.0001f);
                    const float kFalloffFactor = saturate((kCosAngle - gLights[i].cosAngle) / kCosAngleDiff);
                
                    const float4 kLightDiffuse = kObjectDiffuseColor * kLightColor * kNdotL * kAttenuationFactor * kFalloffFactor;
                    const float4 kLightSpecular = kObjectSpecularColor * kLightColor * kSpeclarIntensity * kAttenuationFactor * kFalloffFactor;
               
                    finaleColor += kLightDiffuse + kLightSpecular;
                }
                break;
            
            default:
                break;
        }
    }
    
    // 彩度の取得
    float3 lumiVec = float3(0.299f, 0.587f, 0.114f);
    float luminance = dot(finaleColor.rgb, lumiVec);
    float3 grayScale = float3(luminance, luminance, luminance);
    
    //　あとでCbufferへ
    float3 toCamera = mul(float4(input.worldPosition, 1.0f), gCamera.viewMatrix).xyz;
    float toCameraLen = length(toCamera);
    
    float fogFactor = exp(-pow(toCameraLen * gFogSattus.density, gFogSattus.power));
    
    float thresholdMax = gFogSattus.thresholdEnd > gFogSattus.thresholdStart ? gFogSattus.thresholdEnd : gFogSattus.thresholdStart;
    float fogRatio = smoothstep(gFogSattus.thresholdEnd, thresholdMax, fogFactor);
    fogFactor = saturate(fogFactor);
    if (fogRatio <= 0.0f)
    {
        discard;
    }
    float3 fogColor = float3(0.3f, 0.3f, 0.3f);
    float3 desaturateColor = lerp(grayScale, finaleColor.rgb, fogRatio);
    finaleColor.rgb = lerp(fogColor, desaturateColor, fogRatio);
    finaleColor.a = fogRatio;
    
    output.color = finaleColor;
    //output.color.a = texColor.a;
    return output;
}

// 半透明オブジェクトが消えるので注意
//if (output.color.a <= 0.5f)
//{
//    discard;
//}

// Phong Reflection Model
//float3 reflectDir = reflect(gDirectionalLight.direction, normalize(input.normal));
//float RdotE = dot(reflectDir, toEyeDir);
//float specularIntensity = pow(saturate(RdotE), gMaterial.shininess);