#version 450 core

layout(std140,binding=0) uniform CacaoData {
    mat4 projection;
    mat4 view;
    mat4 transform;
} cacao;

layout(location=0) in vec3 pos;

layout(location=0) out CacaoSky {
    vec3 texCoords;
} V2F;

void main()
{
	V2F.texCoords = pos;
	vec4 skypos = cacao.projection * cacao.view * cacao.transform * vec4(pos, 1.0);
	gl_Position = skypos.xyww;
}
