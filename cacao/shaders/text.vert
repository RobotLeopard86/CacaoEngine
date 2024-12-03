#version 450 core

layout(std140,binding=0) uniform CacaoGlobals {
    mat4 projection;

	//This is unused but required
    mat4 view;
} globals;

layout(push_constant) uniform ObjectData {
	//Unused but required
	mat4 transform;
	
	vec3 color;
} object;

layout(location=0) in vec3 pos;
layout(location=1) in vec2 tc;

layout(location=0) out CacaoTextElem {
    vec2 texCoords;
	vec3 color;
} V2F;

void main() {
    V2F.texCoords = tc;
	V2F.color = object.color;
    gl_Position = globals.projection * vec4(pos.xy, 0.0, 1.0);
}