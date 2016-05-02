#version 410

in vec2 frag_texcoord;
out vec4 frag_color;

uniform sampler2D diffuse;

void main()
{
	frag_color = vec4(1,1,1,1);//texture(diffuse, frag_texcoord);
}