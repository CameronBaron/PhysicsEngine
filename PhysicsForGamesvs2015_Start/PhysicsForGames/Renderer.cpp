#include "Render.h"

#include <vector>
#include "gl_core_4_4.h"
#include "GLFW/glfw3.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

using namespace tinyobj;

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


struct Texture
{
	std::string path;
	unsigned int gl_id;
};

struct TextureBank
{
	std::vector<Texture> textures;
};

TextureBank texture_bank;

bool LoadShaderType(char* filename,
    GLenum shader_type,
    unsigned int* output)
{
    //we want to be able to return if we succeded
    bool succeeded = true;

    //open the shader file
	FILE* shader_file;
	fopen_s(&shader_file, filename, "r");

    //did it open successfully 
    if (shader_file == 0)
    {
        succeeded = false;
    }
    else
    {
        //find out how long the file is
        fseek(shader_file, 0, SEEK_END);
        int shader_file_length = ftell(shader_file);
        fseek(shader_file, 0, SEEK_SET);
        //allocate enough space for the file
        char *shader_source = new char[shader_file_length];
        //read the file and update the length to be accurate
        shader_file_length = fread(shader_source, 1, shader_file_length, shader_file);

        //create the shader based on the type that got passed in
        unsigned int shader_handle = glCreateShader(shader_type);
        //compile the shader
        glShaderSource(shader_handle, 1, &shader_source, &shader_file_length);
        glCompileShader(shader_handle);

        //chech the shader for errors
        int success = GL_FALSE;
        glGetShaderiv(shader_handle, GL_COMPILE_STATUS, &success);
        if (success == GL_FALSE)
        {
            int log_length = 0;
            glGetShaderiv(shader_handle, GL_INFO_LOG_LENGTH, &log_length);
            char* log = new char[log_length];
            glGetShaderInfoLog(shader_handle, log_length, NULL, log);
            printf("%s\n", log);
            delete[] log;
            succeeded = false;
        }
        //only give the result to the caller if we succeeded
        if (succeeded)
        {
            *output = shader_handle;
        }

        //clean up the stuff we allocated
        delete[] shader_source;
        fclose(shader_file);
    }

    return succeeded;
}

bool LoadShader(
    char* vertex_filename,
    char* geometry_filename,
    char* fragment_filename,
    GLuint* result)
{
    bool succeeded = true;

    *result = glCreateProgram();

    unsigned int vertex_shader;

    if (LoadShaderType(vertex_filename, GL_VERTEX_SHADER, &vertex_shader))
    {
        glAttachShader(*result, vertex_shader);
        glDeleteShader(vertex_shader);
    }
    else
    {
        printf("FAILED TO LOAD VERTEX SHADER\n");
    }

    if (geometry_filename != nullptr)
    {
        unsigned int geometry_shader;
        if (LoadShaderType(geometry_filename, GL_GEOMETRY_SHADER, &geometry_shader))
        {
            glAttachShader(*result, geometry_shader);
            glDeleteShader(geometry_shader);
        }
        else
        {
            printf("FAILED TO LOAD GEOMETRY SHADER\n");
        }
    }
    if (fragment_filename != nullptr)
    {
        unsigned int fragment_shader;
        if (LoadShaderType(fragment_filename, GL_FRAGMENT_SHADER, &fragment_shader))
        {
            glAttachShader(*result, fragment_shader);
            glDeleteShader(fragment_shader);
        }
        else
        {
            printf("FAILED TO LOAD FRAGMENT SHADER\n");
        }
    }

    glLinkProgram(*result);

    GLint success;
    glGetProgramiv(*result, GL_LINK_STATUS, &success);
    if (success == GL_FALSE)
    {
        GLint log_length;
        glGetProgramiv(*result, GL_INFO_LOG_LENGTH, &log_length);
        char* log = new char[log_length];
        glGetProgramInfoLog(*result, log_length, 0, log);

        printf("ERROR: STUFF DONE SCREWED UP IN UR SHADER BUDDY!\n\n");
        printf("%s", log);

        delete[] log;
        succeeded = false;
    }

    return succeeded;
}

unsigned int
CreateGLTextureBasic(unsigned char* data, int width, int height, int channels)
{
    GLenum format = 0;
    GLenum src_format = 0;
    switch (channels)
    {
    case 1:
    {
        format = GL_R8;
        src_format = GL_RED;
    }break;
    case 2:
    {
        format = GL_RG8;
        src_format = GL_RG;
    }break;
    case 3:
    {
        format = GL_RGB8;
        src_format = GL_RGB;
    }break;
    case 4:
    {
        format = GL_RGBA8;
        src_format = GL_RGBA;
    }break;
    }

	unsigned int tex_handle;
	glGenTextures(1, (GLuint*)&tex_handle);
	glBindTexture(GL_TEXTURE_2D, tex_handle);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, src_format, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	
	return tex_handle;
}

unsigned LoadGLTextureBasic(const char* filename)
{
    unsigned int result = 0;

	if (filename)
	{
		for (unsigned int i = 0; i < texture_bank.textures.size(); ++i)
		{
			if (texture_bank.textures[i].path == filename)
			{
				result = texture_bank.textures[i].gl_id;
			}
		}

		if (result == 0)
		{
			int width, height, channels;
			unsigned char* data = stbi_load(filename, &width, &height, &channels, 0);
			if (data)
			{
				result = CreateGLTextureBasic(data, width, height, channels);
				stbi_image_free(data);

				Texture tex;
				tex.gl_id = result;
				tex.path = filename;
				texture_bank.textures.push_back(tex);
			}
		}
	}

    return result;
}

Mesh* CreateMeshFromBuffers(SimpleVertex* vertex_data, unsigned int vertex_count, 
						unsigned int *index_data, unsigned int index_count, 
						Material material)
{
	if (!vertex_data || !index_data) return 0;

	//NOTE(aidan): all data for the scene amortized into a single buffer for easy deallocation
	//			This is easy to do as its all just simple POD types
	size_t total_bytes = sizeof(Mesh) + 
						sizeof(SubMesh) + 
						sizeof(SimpleVertex) * vertex_count + 
						sizeof(unsigned int) * index_count;
	void* memory = calloc(total_bytes, 1);

	Mesh* result = (Mesh*)memory;
	SubMesh* sub_mesh = (SubMesh*)(result + 1);
	SimpleVertex* verts = (SimpleVertex*)(sub_mesh + 1);
	unsigned int* indices = (unsigned int*)(verts + vertex_count);

	memcpy(verts, vertex_data, sizeof(SimpleVertex) * vertex_count);
	memcpy(indices, index_data, sizeof(unsigned int) * index_count);

	result->sub_meshes = sub_mesh;
	result->sub_mesh_count = 1;
	result->vertex_count = vertex_count;
	result->vertex_data = verts;
	
	sub_mesh->index_count = index_count;
	sub_mesh->index_data = indices;
	sub_mesh->material = material;
	
	glGenBuffers(1, &result->vbo);
	glBindBuffer(GL_ARRAY_BUFFER, result->vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(SimpleVertex) * result->vertex_count, result->vertex_data, GL_DYNAMIC_DRAW);

	glGenVertexArrays(1, &sub_mesh->vao);
	glBindVertexArray(sub_mesh->vao);

	glGenBuffers(1, &sub_mesh->ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sub_mesh->ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * sub_mesh->index_count, sub_mesh->index_data, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SimpleVertex), 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(SimpleVertex), (void*)(sizeof(float) * 3));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(SimpleVertex), (void*)(sizeof(float) * 6));

	glBindVertexArray(0);

	return result;
}

void FreeMesh(Mesh* mesh)
{
	*mesh = {};
	free(mesh);
}

void 
RebuildVertexBuffer(Mesh* mesh)
{
	glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, mesh->vertex_count * sizeof(SimpleVertex), mesh->vertex_data);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


Renderer::Renderer()
{
    LoadShader("./data/shaders/main_shader.vs", 0, "./data/shaders/main_shader.fs", &main_shader);
}

Renderer::~Renderer()
{}

void Renderer::PushMesh(Mesh* mesh, mat4 transform)
{
    RenderItem item = {};
    item.mesh = mesh;
    item.transform = transform;

    render_queue.push_back(item);
}

void Renderer::RenderAndClear(mat4 view_proj)
{
    glUseProgram(main_shader);

    int view_proj_loc = glGetUniformLocation(main_shader, "view_proj");

    int model_loc = glGetUniformLocation(main_shader, "model");
    int model_view_proj_loc = glGetUniformLocation(main_shader, "model_view_proj");

    int diffuse_loc = glGetUniformLocation(main_shader, "diffuse_tex");
    int normal_loc = glGetUniformLocation(main_shader, "normal_tex");
    int specular_loc = glGetUniformLocation(main_shader, "specular_tex");

    int ambient_location = glGetUniformLocation(main_shader, "ambient_light");
    int light_dir_location = glGetUniformLocation(main_shader, "light_dir");
    int light_color_location = glGetUniformLocation(main_shader, "light_color");
    int spec_pow_location = glGetUniformLocation(main_shader, "specular_power");

    float sq_3 = sqrt(3.f);

    glUniform3f(ambient_location, 0.2f, 0.2f, 0.2f);
    glUniform3f(light_dir_location, sq_3, -sq_3, sq_3);
    glUniform3f(light_color_location, 0.8f, 0.8f, 0.8f);
    glUniform1f(spec_pow_location, 15.0f);

    glUniformMatrix4fv(view_proj_loc, 1, GL_FALSE, (float*)&view_proj);
    glUniform1i(diffuse_loc, 0);
    glUniform1i(normal_loc, 1);
    glUniform1i(specular_loc, 2);

	for (unsigned int i = 0; i < render_queue.size(); ++i)
	{
		RenderItem* item = &render_queue[i];

		mat4 model_view_proj = view_proj * item->transform;

		Mesh* mesh = item->mesh;

		glUniformMatrix4fv(model_loc, 1, GL_FALSE, (float*)&item->transform);
		glUniformMatrix4fv(model_view_proj_loc, 1, GL_FALSE, (float*)&model_view_proj);

		for (unsigned int j = 0; j < mesh->sub_mesh_count; ++j)
		{
			SubMesh* sub_mesh = mesh->sub_meshes + j;

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, sub_mesh->material.diffuse_texture);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, sub_mesh->material.normal_texture);
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, sub_mesh->material.specular_texture);

			glBindVertexArray(sub_mesh->vao);
			glDrawElements(GL_TRIANGLES, sub_mesh->index_count, GL_UNSIGNED_INT, 0);
		}
    }

    glBindVertexArray(0);

	render_queue.clear();
}

Scene LoadSceneFromOBJ(char* dir, char* filename)
{
	Scene result = {};

	if (filename && dir)
	{
		std::string file_path = std::string(dir) + std::string(filename);

		//load OBJ
		std::vector<shape_t> shapes;
		std::vector<material_t> materials;
		std::string err;
		LoadObj(shapes, materials, err, file_path.c_str(), dir);

		std::string dir_str = "";
		if ( dir ) dir_str = dir;

		//compute memory requirements
		int total_vertex_count = 0;
		int total_index_count = 0;
		int total_mesh_count = shapes.size();
		int total_sub_mesh_count = 0;

		for (unsigned int shape = 0; shape < shapes.size(); ++shape)
		{
			total_vertex_count += shapes[shape].mesh.positions.size() / 3;
			total_index_count += shapes[shape].mesh.material_ids.size() * 3;
		}

		//need to know about one more than existing materials
		//as meshes can have be assigned to a 'null' material
		int mp1 = materials.size() + 1;

		int *materials_used = new int[mp1 * shapes.size()];
		memset(materials_used, 0, sizeof(int) * mp1 * shapes.size());

		for (unsigned int mesh_index = 0; mesh_index < shapes.size(); ++mesh_index)
		{
			mesh_t* mesh = &shapes[mesh_index].mesh;

			for (unsigned int j = 0; j < mesh->material_ids.size(); ++j)
			{
				int index = mesh->material_ids[j]+1;
				materials_used[mp1 * mesh_index + index]++;
			}

			for (int j = 0; j < mp1; ++j)
			{
				if (materials_used[mp1 * mesh_index + j] != 0) ++total_sub_mesh_count;
			}
		}
		
		Material default_material = {};
		default_material.diffuse_color = vec3(255, 105, 180);
		default_material.specular_color = vec3(255, 105, 180);

		//All memory for the scene amortized into a single buffer for easy dealocation
		unsigned int total_bytes =
			total_mesh_count * sizeof(Mesh) +
			total_sub_mesh_count * sizeof(SubMesh) +
			total_vertex_count * sizeof(SimpleVertex) +
			total_index_count * sizeof(unsigned int);
		unsigned char* buffer = (unsigned char*)calloc(total_bytes, 1);

		//sub-allocate out the buffer
		Mesh* mesh_buffer = (Mesh*)buffer;
		SubMesh* sub_mesh_buffer = (SubMesh*)(mesh_buffer + total_mesh_count);
		SimpleVertex* vertex_buffer = (SimpleVertex*)(sub_mesh_buffer + total_sub_mesh_count);
		unsigned int *index_buffer = (unsigned int*)(vertex_buffer + total_vertex_count);

		result.meshes = mesh_buffer;
		result.mesh_count = total_mesh_count;

		//loop over meshes to partition out data to the internal format
		for (unsigned int shape_index = 0; shape_index < shapes.size(); ++shape_index)
		{
			Mesh* mesh = mesh_buffer;
			mesh_t* obj_mesh = &shapes[shape_index].mesh;

			mesh->vertex_count = obj_mesh->positions.size() / 3;
			mesh->vertex_data = vertex_buffer;

			if (obj_mesh->positions.size() == mesh->vertex_count * 3)
			{
				for (unsigned int vert = 0; vert < mesh->vertex_count; ++vert) {
					mesh->vertex_data[vert].pos.x = obj_mesh->positions[vert * 3 + 0] / 100.0f;
					mesh->vertex_data[vert].pos.y = obj_mesh->positions[vert * 3 + 1] / 100.0f;
					mesh->vertex_data[vert].pos.z = obj_mesh->positions[vert * 3 + 2] / 100.0f;
				}
			}
			
			if (obj_mesh->texcoords.size() == mesh->vertex_count * 2)
			{
				for (unsigned int vert = 0; vert < mesh->vertex_count; ++vert) {
					mesh->vertex_data[vert].uv.x = obj_mesh->texcoords[vert * 2 + 0];
					mesh->vertex_data[vert].uv.y = obj_mesh->texcoords[vert * 2 + 1];
				}
			}

			if (obj_mesh->normals.size() == mesh->vertex_count * 3)
			{
				for (unsigned int vert = 0; vert < mesh->vertex_count; ++vert) {
					mesh->vertex_data[vert].normal.x = obj_mesh->normals[vert * 3 + 0];
					mesh->vertex_data[vert].normal.y = obj_mesh->normals[vert * 3 + 1];
					mesh->vertex_data[vert].normal.z = obj_mesh->normals[vert * 3 + 2];
				}
			}

			mesh->sub_meshes = sub_mesh_buffer;

			//maps material index to submesh pointer
			SubMesh** sub_mesh_map = new SubMesh*[mp1];
			memset(sub_mesh_map, 0, sizeof(SubMesh*) * mp1);

			for (int j = 0; j < mp1; ++j)
			{
				if (materials_used[mp1 * shape_index + j] != 0)
				{
					sub_mesh_map[j] = sub_mesh_buffer + mesh->sub_mesh_count;
					sub_mesh_map[j]->index_count = materials_used[mp1 * shape_index + j] * 3;

					//assign materials
					if (j == 0)
					{
						sub_mesh_map[j]->material = default_material;
					}
					else
					{
						memcpy(&sub_mesh_map[j]->material.diffuse_color, materials[j - 1].diffuse, sizeof(float) * 3);
						memcpy(&sub_mesh_map[j]->material.specular_color, materials[j - 1].specular, sizeof(float) * 3);

						std::string diffuse_filename = (dir_str + materials[j - 1].diffuse_texname);
						std::string bump_filename = (dir_str + materials[j - 1].bump_texname);
						std::string specular_filename = (dir_str + materials[j - 1].specular_texname);

						sub_mesh_map[j]->material.diffuse_texture = LoadGLTextureBasic(diffuse_filename.c_str());
						sub_mesh_map[j]->material.normal_texture = LoadGLTextureBasic(bump_filename.c_str());
						sub_mesh_map[j]->material.specular_texture = LoadGLTextureBasic(specular_filename.c_str());
					}
					++mesh->sub_mesh_count;
				}
			}

			//sub-allocate index buffers
			for (unsigned int j = 0; j < mesh->sub_mesh_count; ++j)
			{
				mesh->sub_meshes[j].index_data = index_buffer;
				index_buffer += mesh->sub_meshes[j].index_count;
			}
			
			int *index_counts = new int[mesh->sub_mesh_count];
			memset(index_counts, 0, sizeof(int) * mesh->sub_mesh_count);

			for (unsigned int j = 0; j < obj_mesh->material_ids.size(); ++j)
			{
				int id = obj_mesh->material_ids[j];
				SubMesh* sub_mesh = sub_mesh_map[id+1];

				int sub_mesh_index = (int)(sub_mesh - mesh->sub_meshes);

				int index = index_counts[sub_mesh_index];

				sub_mesh->index_data[index + 0] = obj_mesh->indices[j * 3 + 0];
				sub_mesh->index_data[index + 1] = obj_mesh->indices[j * 3 + 1];
				sub_mesh->index_data[index + 2] = obj_mesh->indices[j * 3 + 2];

				index_counts[sub_mesh_index] += 3;
			}

			glGenBuffers(1, &mesh->vbo);
			glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
			glBufferData(GL_ARRAY_BUFFER, sizeof(SimpleVertex) * mesh->vertex_count, mesh->vertex_data, GL_STATIC_DRAW);
			
			for (unsigned int j = 0; j < mesh->sub_mesh_count; ++j)
			{
				SubMesh* sub_mesh = mesh->sub_meshes + j;

				glGenVertexArrays(1, &sub_mesh->vao);
				glBindVertexArray(sub_mesh->vao);

				glGenBuffers(1, &sub_mesh->ibo);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sub_mesh->ibo);
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * sub_mesh->index_count, sub_mesh->index_data, GL_STATIC_DRAW);

				glEnableVertexAttribArray(0);
				glEnableVertexAttribArray(1);
				glEnableVertexAttribArray(2);

				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SimpleVertex), 0);
				glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(SimpleVertex), (void*)(sizeof(float) * 3));
				glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(SimpleVertex), (void*)(sizeof(float) * 6));

				glBindVertexArray(0);
			}

			delete[] index_counts;
			delete[] sub_mesh_map;

			sub_mesh_buffer += mesh->sub_mesh_count;
			mesh_buffer++;
			vertex_buffer += mesh->vertex_count;
		}

		delete[] materials_used;
	}

	return result;
}

void FreeSceneFromOBJ(Scene* scene)
{
	free(scene->meshes);
	*scene = {};
}