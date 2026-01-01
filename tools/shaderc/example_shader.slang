import cacaoshaderbase;

struct VSOut {
    float4 position : SV_Position0;
    float2 texCoords: TexCoord0;
};

struct MaterialData {
	Sampler2D tex;
	float3 tint;
	float yChange;
};
ParameterBlock<MaterialData> mat;

[shader("vertex")] VSOut VS_main(VSIn input) {
    VSOut o = {};
	float3 position = input.position;
	position.y += mat.yChange;
    o.position = mul(cacao.viewProjection, mul(transform, float4(position, 1.0)));
    o.texCoords = input.texCoords;
    return o;
}

[shader("fragment")] float4 FS_main(VSOut input) : SV_Target0 {
    float4 sampled = mat.tex.Sample(input.texCoords);
	return float4(sampled.rgb * mat.tint, sampled.a);
}