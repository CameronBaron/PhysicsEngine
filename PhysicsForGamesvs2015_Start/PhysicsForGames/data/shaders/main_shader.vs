#version 410

layout(location=0) in vec3 position;
layout(location=1) in vec3 normal;
layout(location=2) in vec2 tex_coord;
layout(location=3) in vec3 tangent;
layout(location=4) in vec3 bitangent;

out vec3 frag_normal;
out vec3 frag_position;
out vec3 frag_tangent;
out vec3 frag_bitangent;
out vec2 frag_texcoord;

uniform mat4 view_proj;
uniform mat4 model;
uniform mat4 model_view_proj;

void main()
{
	frag_position = (model * vec4(position, 1)).xyz;
    frag_normal = (model * vec4(normal,1)).xyz;
	frag_tangent = (model * vec4(tangent, 1)).xyz;
	frag_bitangent = (model * vec4(bitangent, 1)).xyz;

	frag_texcoord = tex_coord;
	
	gl_Position = model_view_proj * vec4(position, 1);
}
