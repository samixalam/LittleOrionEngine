#include "GameObject.h"
#include "Application.h"
#include "ModuleCamera.h"
#include "ModuleProgram.h"
#include "ModuleRender.h"
#include "ModuleTexture.h"
#include "Texture.h"

#include "ComponentMaterial.h"
#include "ComponentMesh.h"

GameObject::GameObject()
{
	this->transform = (ComponentTransform*)CreateComponent(Component::ComponentType::TRANSFORM);
}

GameObject::GameObject(const std::string name) :
	name(name)
{
	this->transform = (ComponentTransform*)CreateComponent(Component::ComponentType::TRANSFORM);
}


GameObject::~GameObject()
{
	delete transform;
	for (unsigned int i = 0; i < components.size(); ++i)
	{
		delete components[i];
	}
	for (unsigned int i = 0; i < children.size(); ++i)
	{
		delete children[i];
	}
}

void GameObject::Update()
{
	GLuint shader_program = App->program->texture_program;
	glUseProgram(shader_program);

	transform->Render(shader_program);
	glUniformMatrix4fv(
		glGetUniformLocation(shader_program, "view"),
		1,
		GL_TRUE,
		&App->cameras->view[0][0]
	);
	glUniformMatrix4fv(
		glGetUniformLocation(shader_program, "proj"),
		1,
		GL_TRUE,
		&App->cameras->proj[0][0]
	);

	for (unsigned int i = 0; i < components.size(); ++i)
	{
		if (components[i]->GetType() == Component::ComponentType::MESH)
		{
			ComponentMesh* current_mesh = (ComponentMesh*)components[i];
			int mesh_material_index = current_mesh->material_index;
			const GLuint mesh_texture = GetMaterialTexture(mesh_material_index);
			current_mesh->Render(shader_program, mesh_texture);
		}
	}

	glUseProgram(0);

	for (unsigned int i = 0; i < children.size(); ++i)
	{
		children[i]->Update();
	}

}

void GameObject::AddChild(GameObject *child)
{
	child->parent = this;
	children.push_back(child);
}

Component* GameObject::CreateComponent(const Component::ComponentType type)
{
	Component *created_component;
	switch (type)
	{
	case Component::ComponentType::MATERIAL:
		created_component = App->texture->CreateComponentMaterial();
		break;

	case Component::ComponentType::MESH:
		created_component = App->renderer->CreateComponentMesh();
		break;

	case Component::ComponentType::TRANSFORM:
		created_component = new ComponentTransform();
		break;

	default:
		APP_LOG_ERROR("Error creating component. Incorrect component type.");
		return nullptr;
	}

	created_component->owner = this;
	components.push_back(created_component);

	return created_component;
}

const GLuint GameObject::GetMaterialTexture(const int material_index)
{
	for (unsigned int i = 0; i < components.size(); ++i)
	{
		if (components[i]->GetType() == Component::ComponentType::MATERIAL)
		{
			ComponentMaterial* current_material = (ComponentMaterial*)components[i];
			if (current_material->index == material_index)
			{
				return current_material->texture->opengl_texture;
			}
		}
	}

	return 0;
}


