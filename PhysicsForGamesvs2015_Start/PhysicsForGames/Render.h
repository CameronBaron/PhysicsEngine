#ifndef _RENDER_H_
#define _RENDER_H_

#define GLM_SWIZZLE
#include "glm/glm.hpp"

#include <vector>

using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::mat4;

struct Material
{
    int diffuse_texture;
    int normal_texture;
    int specular_texture;

    vec3 diffuse_color;
    vec3 specular_color;
};

struct SimpleVertex
{
    vec3 pos;
    vec3 normal;
    vec2 uv;
};

struct SubMesh
{
	Material material;

    unsigned int ibo;
    unsigned int vao;
	
	unsigned int* index_data;
	unsigned int index_count;
};

struct Mesh
{
	unsigned int sub_mesh_count;
	SubMesh* sub_meshes;

	unsigned int vbo;

	unsigned int vertex_count;
	SimpleVertex* vertex_data;
};

struct Scene
{
	Mesh* meshes;
	unsigned int mesh_count;
};


Scene LoadSceneFromOBJ(char* dir, char* filename);
void FreeSceneFromOBJ(Scene* scene);

unsigned int LoadGLTextureBasic(const char * path);


Mesh *CreateMeshFromBuffers(SimpleVertex* vertex_data, unsigned int vertex_count, unsigned int *index_data, unsigned int index_count, Material material);
//NOTE(aidan): This should only be called on meshes created from the
//CreateMeshFromBuffers function. DO NOT call on the meshes in a scene
void FreeMesh(Mesh* mesh);
void RebuildVertexBuffer(Mesh* mesh);


class Renderer
{
public:
    Renderer();
    ~Renderer();

    void PushMesh(Mesh* mesh, mat4 transform);
    void RenderAndClear(mat4 view_proj);

	struct RenderItem
	{
		Mesh* mesh;
		mat4 transform;
	};

	std::vector<RenderItem> render_queue;

    unsigned int main_shader;
};

#endif	