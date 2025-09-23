#version 450 core

layout(location = 0) out vec4 color;

layout(location = 0) in Vertex2Fragment {
	vec2 tex;
} V2F;

layout(binding=2) uniform sampler2D texSample;

void main() {
	color = texture(texSample, V2F.tex);
}