import cacaoengine;

struct MaterialParameters {
	Sampler2D texture;
};
ParameterBlock<MaterialParameters> material;

void CacaoSurfaceMain(const Cacao::SurfaceInput input, inout Cacao::SurfaceSample output) {
	output.albedo = material.texture.Sample(input.texCoords);
}
