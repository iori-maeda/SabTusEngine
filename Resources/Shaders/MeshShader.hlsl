struct VertexInput
{
    float4 position : POSITION;
    float3 normal : NORMAL;
    float2 texcoord : TEXCOORD0;
};

struct VertexOutput
{
    float4 worldPos : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
};

struct MeshOutput
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
};

#define PLANE_RES 8

[numthreads(1, 1, 1)]
[outputtopology("triangle")]
void main(
    uint3 dispatchThreadID : SV_DispatchThreadID,
    out indices uint3 inds[PLANE_RES * PLANE_RES * 2],
    out vertices MeshOutput verts[(PLANE_RES + 1) * (PLANE_RES + 1)]
)
{
    SetMeshOutputCounts((PLANE_RES + 1) * (PLANE_RES + 1), PLANE_RES * PLANE_RES * 2);
    
    for (uint y = 0; y <= PLANE_RES; ++y)
    {
        for (uint x = 0; x <= PLANE_RES; ++x)
        {
            uint index = y * (PLANE_RES + 1) + x;
            verts[index].position = float4(x / (float) PLANE_RES, 0.0f, y / (float) PLANE_RES, 1.0f);
            verts[index].normal = float3(0.0f, 1.0f, 0.0f);
            verts[index].uv = float2(x / (float) PLANE_RES, y / (float) PLANE_RES);
        }
    }
     // インデックスの生成（2つの三角形で1 quad）
    uint tri = 0;
    for (uint row = 0; row < PLANE_RES; ++row)
    {
        for (uint column = 0; column < PLANE_RES; ++column)
        {
            uint i0 = row * (PLANE_RES + 1) + column;
            uint i1 = i0 + 1;
            uint i2 = i0 + (PLANE_RES + 1);
            uint i3 = i2 + 1;

            inds[tri++] = uint3(i0, i1, i2); // 1枚目の三角形
            inds[tri++] = uint3(i1, i3, i2); // 2枚目の三角形
        }
    }

}