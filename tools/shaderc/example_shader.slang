import cacaoengine;

struct MaterialParameters {
	Sampler2D tex;
	float3 tint;
	float yChange;
};
ParameterBlock<MaterialParameters> material;

void CacaoVertexPreprocess(const Cacao::VSIn3D input, inout Cacao::VSIn3D output) {
	output.position.y += material.yChange;
}

void CacaoSurfaceMain(const Cacao::SurfaceInput input, inout Cacao::SurfaceSample output) {
	float4 sampled = material.tex.Sample(input.texCoords);
	output.albedo = float4(sampled.rgb * material.tint, sampled.a);
}
