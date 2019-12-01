#include "ModuleScene.h"
#include "Application.h"
#include "ModuleRender.h"
#include "ModuleCamera.h"

#include "imgui.h"
#include <FontAwesome5/IconsFontAwesome5.h>
#include <algorithm>

bool ModuleScene::Init()
{
	root = new GameObject();
	
	return true;
}

bool ModuleScene::CleanUp()
{
	delete root;

	return true;
}

GameObject* ModuleScene::CreateGameObject()
{
	std::string created_game_object_name = hierarchy.GetNextGameObjectName();
	std::unique_ptr<GameObject> created_game_object = std::make_unique<GameObject>(created_game_object_name);
	created_game_object->SetParent(root);

	GameObject * created_game_object_ptr = created_game_object.get();
	game_objects_ownership.emplace_back(std::move(created_game_object));
	return created_game_object_ptr;
}

void ModuleScene::RemoveGameObject(GameObject * gameObjectToRemove) {
	auto it = std::remove_if(game_objects_ownership.begin(), game_objects_ownership.end(), [gameObjectToRemove](auto const & gameObject)
	{
		return gameObject.get() == gameObjectToRemove;
	});
	game_objects_ownership.erase(it);
}

void ModuleScene::Render() const
{
	root->Update();
	root->Render();
}

GameObject* ModuleScene::GetRoot() const
{
	return root;
}

void ModuleScene::ShowSceneWindow()
{
	if (ImGui::Begin(ICON_FA_TH " Scene"))
	{
		scene_window_is_hovered = ImGui::IsWindowHovered();
		float imgui_window_width = ImGui::GetWindowWidth();
		float imgui_window_height = ImGui::GetWindowHeight();
		App->cameras->SetAspectRatio(imgui_window_width / imgui_window_height);
		App->renderer->GenerateFrameTexture(imgui_window_width, imgui_window_height);

		ImGui::GetWindowDrawList()->AddImage(
			(void *)App->renderer->frame_texture,
			ImVec2(ImGui::GetCursorScreenPos()),
			ImVec2(
				ImGui::GetCursorScreenPos().x + imgui_window_width,
				ImGui::GetCursorScreenPos().y + imgui_window_height
			),
			ImVec2(0, 1),
			ImVec2(1, 0)
		);
	}
	if (App->cameras->MovementEnabled() && scene_window_is_hovered)
	{
		ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);
	}
	ImGui::End();
}