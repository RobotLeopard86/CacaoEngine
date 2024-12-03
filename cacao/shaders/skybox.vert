#version 450 core

layout(std140,binding=0) uniform CacaoGlobals {
    mat4 projection;
    mat4 view;
} globals;

layout(push_constant) uniform ObjectData {
	mat4 transform;
} object;

layout(location=0) in vec3 pos;

layout(location=0) out CacaoSky {
    vec3 texCoords;
} V2F;

void main() {
	V2F.texCoords = pos;
	mat4 view = mat4(mat3(globals.view));
	vec4 skypos = globals.projection * view * object.transform * vec4(pos, 1.0);
	gl_Position = skypos.xyww;
}
