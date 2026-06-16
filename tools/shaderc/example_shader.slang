import cacaoengine;

struct MaterialParameters {
	Sampler2D aVeryCoolTexture;
	float3 tint;
	float yChange;
	Sampler2D anotherVeryCoolTexture;
};
ParameterBlock<MaterialParameters> material;

void CacaoVertexPreprocess(const Cacao::VSIn3D input, inout Cacao::VSIn3D output) {
	output.position.y += material.yChange;
}

void CacaoSurfaceMain(const Cacao::SurfaceInput input, inout Cacao::SurfaceSample output) {
	float4 sampled = material.aVeryCoolTexture.Sample(input.texCoords);
	float4 anotherSampled = material.anotherVeryCoolTexture.Sample(input.texCoords / 2);
	output.albedo = float4((sampled.rgb * material.tint) + anotherSampled.gba, sampled.a / (anotherSampled.a - 0.22));
}
