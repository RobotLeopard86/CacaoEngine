import cacaoengine;
import cacaousercode;

Cacao::VSOut3D _GenOutput(Cacao::VSIn3D input) {
	Cacao::VSOut3D out = {};
	float4 worldPos = mul(Cacao::transform.transformMatrix, float4(input.position, 1.0));
	out.surfaceIn.worldPos = worldPos.xyz / worldPos.w;
	out.gpuPosition = mul(Cacao::cam.viewProjectionMatrix, worldPos);
	out.surfaceIn.localPos = input.position;
	out.surfaceIn.texCoords = input.texCoords;
	out.surfaceIn.normal = normalize(mul(Cacao::transform.normalMatrix, input.normal));
	out.surfaceIn.tangent = float4(normalize(mul(Cacao::transform.normalMatrix, input.tangent.xyz)), input.tangent.w * Cacao::transform.handedness);
	return out;
}

Cacao::VSOut2D _GenOutput2D(Cacao::VSIn2D input) {
	Cacao::VSOut2D out = {};
	out.gpuPosition = mul(Cacao::cam.projectionMatrix, float4(input.position, 0.0, 1.0));
	out.canvasIn.position = input.position;
	out.canvasIn.texCoords = input.texCoords;
	return out;
}

[shader("vertex")]
Cacao::VSOut3D Cacao_NoProcess3D_VSmain(Cacao::VSIn3D input) {
	//No vertex processing, just generate output and return it
	return _GenOutput(input);
}

[shader("vertex")]
Cacao::VSOut3D Cacao_Preprocess3D_VSmain(Cacao::VSIn3D input) {
	Cacao::VSIn3D modified = input;
	CacaoVertexPreprocess(input, modified);
	return _GenOutput(modified);
}

[shader("vertex")]
Cacao::VSOut3D Cacao_Postprocess3D_VSmain(Cacao::VSIn3D input) {
	Cacao::VSOut3D base = _GenOutput(input);
	Cacao::VSOut3D out = base;
	CacaoVertexPostprocess(base, out);
	return out;
}

[shader("vertex")]
Cacao::VSOut3D Cacao_PrePostprocess3D_VSmain(Cacao::VSIn3D input) {
	Cacao::VSIn3D modified = input;
	CacaoVertexPreprocess(input, modified);
	Cacao::VSOut3D base = _GenOutput(modified);
	Cacao::VSOut3D out = base;
	CacaoVertexPostprocess(base, out);
	return out;
}

[shader("vertex")]
Cacao::VSOut3D Cacao_Custom3D_VSmain(Cacao::VSIn3D input) {
	Cacao::VSOut3D out = {};
	CacaoVertexCustom(input, out);
	return out;
}

[shader("vertex")]
Cacao::VSOut2D Cacao_NoProcess2D_VSmain(Cacao::VSIn2D input) {
	//No vertex processing, just generate output and return it
	return _GenOutput2D(input);
}

[shader("vertex")]
Cacao::VSOut2D Cacao_Preprocess2D_VSmain(Cacao::VSIn2D input) {
	Cacao::VSIn2D modified = input;
	CacaoVertexPreprocess2D(input, modified);
	return _GenOutput2D(modified);
}

[shader("vertex")]
Cacao::VSOut2D Cacao_Custom2D_VSmain(Cacao::VSIn2D input) {
	Cacao::VSOut2D out = {};
	CacaoVertexCustom2D(input, out);
	return out;
}
