#version 450 core

layout(location = 0) out vec4 color;

layout(location = 0) in Vertex2Fragment {
	vec4 pos;
} V2F;

void main() {
	color = vec4(V2F.pos.z, V2F.pos.x, V2F.pos.y, 1.0f);
}