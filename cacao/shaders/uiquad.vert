#version 450 core

layout(std140,binding=0) uniform CacaoGlobals {
    mat4 projection;
	
	//Unusued but required
    mat4 view;
} globals;

//This is unused as positions come in pixels, but it's required
layout(std140,binding=1) uniform CacaoLocals {
    mat4 transform;
} locals;

layout(location=0) in vec3 pos;

layout(location=0) out CacaoUIQuad {
    vec2 texCoords;
} V2F;

void main()
{
	V2F.texCoords = pos.xy;
	gl_Position = globals.projection * vec4(pos.xy, 0.0, 1.0);
}
