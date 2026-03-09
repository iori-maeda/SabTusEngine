#include "Basic3D.hlsli"
#include "LightFunction.hlsli"
#include "Basic3DStructs.PS.hlsli"

ConstantBuffer<MeshMaterial> gMeshMaterial : register(b0);
ConstantBuffer<Essential> gEssential : register(b1);
ConstantBuffer<Camera> gCamera : register(b2);
ConstantBuffer<ObjectMaterial> gObjectMaterial : register(b3);
ConstantBuffer<FogStatus> gFogSattus : register(b4);

StructuredBuffer<LightStatus> gLights : register(t0);

Texture2D<float4> gTexture : register(t1);
Texture2D<float4> gNormalTexture : register(t2);
Texture2D<float4> gRoughnessTextureOrARMTexture : register(t3);
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

    float4 arbedColor = gTexture.Sample(gSampler, input.uv);
    float4 texColor = gRoughnessTextureOrARMTexture.Sample(gSampler, input.uv);
    
    float4 normalTexColor = gNormalTexture.Sample(gSampler, input.uv);
    float3 normal = normalize(input.normal);
    float3 tangent = normalize(input.tangent);
    // 再直行させるための計算式
    tangent = normalize(tangent - dot(tangent, normal) * normal);
    
    float3 binormal = input.binormal;
    float3x3 tangentBinormalMat = float3x3(tangent, binormal, normal);
    
    float3 newNormal = (normalTexColor * 2.0f - 1.0f).rgb;
    
    newNormal = normalize(mul(newNormal, tangentBinormalMat));
	
    if (arbedColor.a <= 0.0f)
    {
        discard;
    }
    
    // Base Color
    const float4 kObjectAmbientColor = gMeshMaterial.Ka * arbedColor * gObjectMaterial.color;
    const float4 kObjectDiffuseColor = gMeshMaterial.Kd * arbedColor * gObjectMaterial.color;
    const float4 kObjectSpecularColor = gMeshMaterial.Ks * gObjectMaterial.color;
    
    const float4 useAmbientColor = length(gMeshMaterial.Ka) >= length(gMeshMaterial.Kd) ? kObjectAmbientColor * 0.5f : kObjectAmbientColor;
    
    if (gObjectMaterial.enableLighting == 0)
    {
        output.color = kObjectAmbientColor + kObjectDiffuseColor + kObjectSpecularColor;
        output.color.a = arbedColor.a;
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
    
                    // ラフネスの取得
                    float roughness = texColor[gMeshMaterial.roughnessChannel];
                    // メタリックの取得
                    float metallic = gMeshMaterial.isUseArmTex ? texColor[gMeshMaterial.metallicChannel] : gMetallicTexture.Sample(gSampler, input.uv)[gMeshMaterial.metallicChannel];
                    // アルベド
                    float4 baseColor = arbedColor;
                
                    float3 diffuseColor = baseColor.rgb * (1.0f - metallic);
                    
                    float3 specColor = lerp(float3(0.04f, 0.04f, 0.04f), baseColor.rgb, metallic);
                    
                    // 移行のため近似値による実装
                    float n = 2.0f / (roughness * roughness + 0.0001f) - 2.0f;
                    float normalization = (n + 8.0f) / (8.0f * 3.1415926535f);
                    float roughnessIntensity = pow(saturate(kNdotH), n) * normalization;
                    const float kSpeclarIntensity = gObjectMaterial.useRoughness ? roughnessIntensity : pow(saturate(kNdotH), gMeshMaterial.shininess);
                    const float4 kLightDiffuse = float4(diffuseColor, 1.0f) * kLightColor * kNdotL;
                    const float4 kLightSpecular = float4(specColor, 1.0f) * kLightColor * kSpeclarIntensity;
                
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