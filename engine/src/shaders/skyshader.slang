import cacaoengine;

struct MaterialParameters {
	SamplerCube skyTex;
}
ParameterBlock<MaterialParameters> material;

void CacaoVertexCustom(const Cacao::VSIn3D input, inout Cacao::VSOut3D output) {
	float3x3 upper3x3 = float3x3(Cacao::cam.viewMatrix);
	float4x4 adjustedView = float4x4(
		float4(upper3x3[0], 0.0),
		float4(upper3x3[1], 0.0),
		float4(upper3x3[2], 0.0),
		float4(0.0, 0.0, 0.0, 1.0));
	float4 skypos = mul(Cacao::cam.projectionMatrix, mul(adjustedView, mul(Cacao::transform.transformMatrix, float4(input.position, 1.0))));
	output.gpuPosition = skypos.xyww;
	output.surfaceIn.localPos = input.position;
}

void CacaoSurfaceMain(const Cacao::SurfaceInput input, inout Cacao::SurfaceSample output) {
	output.albedo = material.skyTex.Sample(input.localPos);
}
