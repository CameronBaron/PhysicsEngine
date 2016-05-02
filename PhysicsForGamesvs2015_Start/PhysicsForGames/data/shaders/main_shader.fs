#version 410

in vec3 frag_normal;
in vec3 frag_position;
in vec3 frag_tangent;
in vec3 frag_bitangent;
in vec2 frag_texcoord;

out vec4 frag_color;

uniform vec3 ambient_light;
uniform vec3 light_dir;
uniform vec3 light_color;
uniform float specular_power;

uniform vec3 eye_pos;
uniform vec3 camera_dir;

uniform sampler2D diffuse_tex;
uniform sampler2D normal_tex;
uniform sampler2D specular_tex;

void main()
{
	mat3 TBN = mat3(normalize(frag_tangent), 
					normalize(frag_bitangent),
					normalize(frag_normal));
	
	vec3 sampled_normal = texture(normal_tex, frag_texcoord).xyz;
	vec3 adjusted_normal = sampled_normal * 2 - 1;

	vec3 N = normalize( TBN * adjusted_normal );

	vec3 material_color = texture(diffuse_tex, vec2(frag_texcoord.x*2,1) - frag_texcoord).xyz;

	vec3 L = normalize(light_dir);

	vec3 ambient = material_color * ambient_light;

	float d = max(0.0, dot(N, -L));
	vec3 diffuse = vec3(d) * light_color * material_color;

	vec3 E = normalize(eye_pos - frag_position);
	vec3 R = reflect(L, N);

	float s = max(0, dot(R, E));

	vec3 material_specular = texture(specular_tex, frag_texcoord).xyz;
	s = pow(s, specular_power);
	vec3 specular = vec3(s) * light_color * material_specular;
    
	frag_color = vec4(material_color,1);// vec4(ambient + diffuse,1);
}