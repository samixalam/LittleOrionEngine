#include "ModuleModelLoader.h"
#include "Globals.h"
#include "Application.h"
#include "ModuleCamera.h"
#include "ModuleTexture.h"
#include "ModuleScene.h"
#include "GameObject.h"
#include "ComponentMaterial.h"
#include "ComponentMesh.h"

#include "assimp/DefaultLogger.hpp"
#include <assimp/cimport.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/material.h>
#include <assimp/mesh.h>

// Called before render is available
bool ModuleModelLoader::Init()
{
	APP_LOG_SECTION("************ Module ModelLoader Init ************");

	Assimp::DefaultLogger::create("", Assimp::Logger::VERBOSE);
	Assimp::DefaultLogger::get()->attachStream(new AssimpStream(Assimp::Logger::Debugging), Assimp::Logger::Debugging);
	Assimp::DefaultLogger::get()->attachStream(new AssimpStream(Assimp::Logger::Info), Assimp::Logger::Info);
	Assimp::DefaultLogger::get()->attachStream(new AssimpStream(Assimp::Logger::Err), Assimp::Logger::Err);
	Assimp::DefaultLogger::get()->attachStream(new AssimpStream(Assimp::Logger::Warn), Assimp::Logger::Warn);
	LoadModel(HOUSE_MODEL_PATH);
	return true;
}

// Called before quitting
bool ModuleModelLoader::CleanUp()
{
	UnloadCurrentModel();
	Assimp::DefaultLogger::kill();
	return true;
}


void ModuleModelLoader::UnloadCurrentModel()
{
	aiReleaseImport(scene);
}

bool ModuleModelLoader::LoadModel(const char *new_model_file_path)
{
	APP_LOG_INIT("Loading model %s.", new_model_file_path);
	scene = aiImportFile(new_model_file_path, aiProcessPreset_TargetRealtime_MaxQuality);
	if (scene == NULL)
	{
		const char *error = aiGetErrorString();
		APP_LOG_ERROR("Error loading model %s ", new_model_file_path);
		APP_LOG_ERROR(error);
		return false;
	}

	GameObject *model_gamobject = App->scene->CreateGameObject();

	APP_LOG_INFO("Loading model materials.");
	std::string model_base_path = GetModelBasePath(new_model_file_path);
	for (unsigned i = 0; i < scene->mNumMaterials; ++i)
	{
		APP_LOG_INFO("Loading material %d.", i);
		Texture *material_texture = LoadMaterialData(scene->mMaterials[i], model_base_path);
		ComponentMaterial *material_component = (ComponentMaterial*)model_gamobject->CreateComponent(Component::ComponentType::MATERIAL);

		material_component->index = i;
		material_component->texture = material_texture;
	}
	APP_LOG_INFO("Model materials loaded correctly.");


	APP_LOG_INFO("Loading model meshes.");
	for (unsigned int i = 0; i < scene->mNumMeshes; ++i)
	{
		APP_LOG_INFO("Loading mesh %d", i);
		ComponentMesh *mesh_component = (ComponentMesh*)model_gamobject->CreateComponent(Component::ComponentType::MESH);
		LoadMeshData(scene->mMeshes[i], mesh_component);
	}
	APP_LOG_INFO("Model meshes loaded correctly.");


	APP_LOG_INFO("Computing model bounding box.");
	//current_model->ComputeBoundingBox(); TODO: This

	//App->cameras->Center(*current_model->bounding_box);

	APP_LOG_SUCCESS("Model %s loaded correctly.", new_model_file_path);
	return true;
}

void ModuleModelLoader::LoadMeshData(const aiMesh *mesh, ComponentMesh *mesh_component) const
{
	std::vector<ComponentMesh::Vertex> vertices;
	APP_LOG_INFO("Loading mesh vertices");
	for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
	{
		ComponentMesh::Vertex vertex;
		vertex.position = float3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
		vertex.tex_coords = float2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
		vertices.push_back(vertex);
	}

	APP_LOG_INFO("Loading mesh indices");
	std::vector<unsigned int> indices;
	for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
	{
		assert(mesh->mFaces[i].mNumIndices == 3);
		indices.push_back(mesh->mFaces[i].mIndices[0]);
		indices.push_back(mesh->mFaces[i].mIndices[1]);
		indices.push_back(mesh->mFaces[i].mIndices[2]);
	}

	APP_LOG_INFO("Mesh uses material %d", mesh->mMaterialIndex);

	mesh_component->LoadMesh(vertices, indices, mesh->mMaterialIndex);
}

Texture* ModuleModelLoader::LoadMaterialData(const aiMaterial *material, const std::string model_base_path)
{
	aiString file;
	aiTextureMapping mapping = aiTextureMapping_UV;
	material->GetTexture(aiTextureType_DIFFUSE, 0, &file, &mapping, 0);

	Texture *material_texture;

	APP_LOG_INIT("Loading material texture in described path %s.", file.data);
	material_texture = App->texture->LoadTexture(file.data);
	if (material_texture != nullptr)
	{
		APP_LOG_SUCCESS("Material loaded correctly.");
		return material_texture;
	}

	std::string texture_file_name = GetTextureFileName(file.data);

	std::string textures_path = model_base_path + texture_file_name;
	APP_LOG_INIT("Loading material texture in model folder path %s.", model_base_path.c_str());
	material_texture = App->texture->LoadTexture(textures_path.c_str());
	if (material_texture != nullptr)
	{
		APP_LOG_SUCCESS("Material loaded correctly.");
		return material_texture;
	}

	textures_path = std::string(TEXTURES_PATH) + texture_file_name;
	APP_LOG_INIT("Loading material texture in textures folder %s.", textures_path.c_str());
	material_texture = App->texture->LoadTexture(textures_path.c_str());
	if (material_texture != nullptr)
	{
		APP_LOG_SUCCESS("Material loaded correctly.");
		return material_texture;
	}

	return nullptr;
}

std::string ModuleModelLoader::GetModelBasePath(const char *model_file_path) const
{
	std::string file_string = std::string(model_file_path);
	
	std::size_t found = file_string.find_last_of("/\\");
	std::string model_base_path = file_string.substr(0, found + 1);

	return model_base_path;
}

std::string ModuleModelLoader::GetTextureFileName(const char *texture_file_path) const
{
	std::string texture_path_string = std::string(texture_file_path);

	std::size_t found = texture_path_string.find_last_of("/\\");
	if (found == std::string::npos)
	{
		return texture_path_string;
	}
	else
	{
		std::string texture_filename = texture_path_string.substr(found, texture_path_string.length());

		return texture_filename;
	}
}