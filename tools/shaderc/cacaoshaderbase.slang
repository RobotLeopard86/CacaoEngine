struct VSIn {
    float3 position  : Position0;
    float2 texCoords : TexCoord0;
    float3 normal    : Normal0;
    float3 tangent   : Tangent0;
    float3 bitangent : Bitangent0;
};

struct CacaoGlobalData {
    float4x4 projection;
    float4x4 view;
};
ParameterBlock<CacaoGlobalData> cacao;