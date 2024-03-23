#version 450 core

layout(location=0) out vec4 color;

layout(location=0) in VS_Out {
	vec4 pos;
} IN;

void main() {
	color = IN.pos;
}