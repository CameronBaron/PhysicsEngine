#version 410

layout(location=0) in vec4 position;
layout(location=1) in vec2 texcoord;

out vec2 frag_texcoord;

uniform mat4 projection_view;

void main()
{
	frag_texcoord = texcoord;
	gl_Position = projection_view * position;
}