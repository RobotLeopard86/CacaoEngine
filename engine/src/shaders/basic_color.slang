import cacaoengine;

struct MaterialParameters {
	float4 color;
};
ParameterBlock<MaterialParameters> material;

void CacaoSurfaceMain(const Cacao::SurfaceInput input, inout Cacao::SurfaceSample output) {
	output.albedo = material.color;
}
