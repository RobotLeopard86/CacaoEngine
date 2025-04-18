#version 450 core

layout(location = 0) in vec3 position;

layout(std140,binding=0) uniform CacaoGlobals {
    mat4 projection;
    mat4 view;
} globals;

layout(push_constant) uniform Transformation {
    mat4 transform;
};

layout(location = 0) out Vertex2Fragment {
	vec4 pos;
} V2F;

void main() {
	gl_Position = globals.projection * globals.view * transform * vec4(position, 1.0);
	V2F.pos = gl_Position;
}