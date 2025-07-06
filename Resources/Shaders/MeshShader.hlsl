struct VertexInput
{
    float3 position : POSITION;
    float4 color;
};

struct VertexOutput
{
    float4 position : SV_Position;
    float4 color : COLOR0;
};

struct PrimitiveOutput
{
    uint primitiveId : INDEX0;
};

StructuredBuffer<VertexInput> tVertices : register(t0);
StructuredBuffer<uint3> tIndecies : register(t1);


///
/// Entry Point Main Function
///
[numthreads(32, 1, 1)]
[outputtopology("triangle")]
void main(
   uint groupIndex : SV_GroupIndex,
   out vertices VertexOutput verts[3], // 頂点必須
   out indices uint3 tris[1], // インデックスも必須
   out primitives PrimitiveOutput prims[1] // オプション

)
{
    SetMeshOutputCounts(3, 1);
    
    if (groupIndex < 1)
    {
        tris[groupIndex] = tIndecies[groupIndex];   // 頂点インデックスの設定
        prims[groupIndex].primitiveId = groupIndex; // プリミティブインデックス設定
    }
    
    if (groupIndex < 3)
    {
        VertexOutput vOut;
        vOut.position = float4(tVertices[groupIndex].position, 1.0f);
        vOut.color = tVertices[groupIndex].color;
        
        verts[groupIndex] = vOut;  // 出力
    }
}