#include "BasicParticle.hlsli"
#include "Math.hlsli"

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

struct ParticleEssential
{
    CameraData camera;
    int useBillboard;
};

StructuredBuffer<ParticleForGPU> gParticle : register(t0);
ConstantBuffer<ParticleEssential> gParticleEssential : register(b0);

struct Input
{
    float4 position : POSITION0;
    float2 uv : TEXCOORD0;
};

VertexOutput main(Input input, uint instanceId : SV_InstanceID)
{
    VertexOutput output;
    CameraData camera = gParticleEssential.camera;
    
    if (gParticleEssential.useBillboard)
    {
        // カメラの方向取得
        float3 cameraRight = float3(camera.viewMat[0][0], camera.viewMat[1][0], camera.viewMat[2][0]);
        float3 cameraUp = float3(camera.viewMat[0][1], camera.viewMat[1][1], camera.viewMat[2][1]);
        // ビルボードによる位置のオフセット
        float3 worldPos =
        gParticle[instanceId].transform.potision 
        + cameraRight * input.position.x * gParticle[instanceId].transform.scale.x 
        + cameraUp * input.position.y * gParticle[instanceId].transform.scale.y;
        
        output.position = mul(float4(worldPos, 1.0f), mul(camera.viewMat, camera.projMat));

    }
    else
    {
        // ワールド行列計算
        float4x4 scaleMat = ScaleMatrix(gParticle[instanceId].transform.scale);
        float4x4 rotateMat = RotateMatrix(gParticle[instanceId].transform.rotation);
        float4x4 translateMat = TranslateMat(gParticle[instanceId].transform.potision);
        float4x4 worldMat = mul(scaleMat, mul(rotateMat, translateMat));
        // WVP
        float4x4 WVP = mul(worldMat, mul(camera.viewMat, camera.projMat));
        // Billboard適用なら計算を変える
        output.position = mul(input.position, WVP);
    }
    
    output.uv = input.uv;
    output.color = gParticle[instanceId].color;
    return output;
}