import cacaoshaderbase;

struct MaterialParameters {
	Sampler2D tex;
	float3 tint;
	float yChange;
};
ParameterBlock<MaterialParameters> material;

void CacaoVertexMain(Cacao::VSIn3D input, inout Cacao::VSOut output) {
	float3 position = input.position;
	position.y += material.yChange;
	output.surfaceIn.objWorldPos += material.yChange;
	output.outPosition = mul(Cacao::cam.viewProjectionMatrix, mul(Cacao::transform.transformMatrix, float4(position, 1.0)));
}

void CacaoSurfaceMain(Cacao::SurfaceInput input, inout Cacao::SurfaceSample output) {
	float4 sampled = material.tex.Sample(input.texCoords);
	output.albedo = float4(sampled.rgb * material.tint, sampled.a);
}
