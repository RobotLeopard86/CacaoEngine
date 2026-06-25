//This file exists purely to provide stub definitions of all allowed user code functions for the Slang LSP
//No actual logic should be put here!

import cacaoengine;

void CacaoVertexPreprocess(const Cacao::VSIn3D input, inout Cacao::VSIn3D output) {}
void CacaoVertexPostprocess(const Cacao::VSOut3D input, inout Cacao::VSOut3D output) {}
void CacaoVertexCustom(const Cacao::VSIn3D input, inout Cacao::VSOut3D output) {}
void CacaoVertexPreprocess2D(const Cacao::VSIn2D input, inout Cacao::VSIn2D output) {}
void CacaoVertexCustom2D(const Cacao::VSIn2D input, inout Cacao::VSOut2D output) {}
void CacaoSurfaceMain(const Cacao::SurfaceInput input, inout Cacao::SurfaceSample output) {}
float4 CacaoCanvasMain(const Cacao::CanvasInput input) {}
