#version 450 core

layout(location=0) out vec4 color;

layout(location=0) in CacaoUIQuad {
    vec2 texCoords;
} V2F;

layout(binding=1) uniform sampler2D uiTex;

void main()
{
	color = texture(uiTex, V2F.texCoords);
}
