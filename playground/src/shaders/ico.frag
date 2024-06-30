#version 450 core

layout(location = 0) out vec4 color;

layout(location = 0) in Vertex2Fragment {
	vec4 pos;
} V2F;

void main() {
	color = V2F.pos;
}