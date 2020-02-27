#include "PanelResourcesExplorer.h"

#include "Main/Application.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <SDL/SDL.h>
#include <FontAwesome5/IconsFontAwesome5.h>
#include <FontAwesome5/IconsFontAwesome5Brands.h>
#include <algorithm>

PanelResourcesExplorer::PanelResourcesExplorer()
{
	opened = true;
	enabled = true;
	window_name = ICON_FA_FOLDER " Resources Explorer";
}

void PanelResourcesExplorer::Render()
{
	if (ImGui::Begin(ICON_FA_FOLDER " Resources Explorer", &opened))
	{
		hovered = ImGui::IsWindowHovered();

		resources_explorer_dockspace_id = ImGui::GetID("ResourcesExplorerDockspace");
		bool initialized = ImGui::DockBuilderGetNode(resources_explorer_dockspace_id) != NULL;

		ImGuiDockNodeFlags resources_explorer_dockspace_flags = ImGuiDockNodeFlags_None;
		resources_explorer_dockspace_flags |= ImGuiDockNodeFlags_NoWindowMenuButton;
		resources_explorer_dockspace_flags |= ImGuiDockNodeFlags_NoCloseButton;
		
		ImGui::DockSpace(resources_explorer_dockspace_id, ImVec2(0,0), resources_explorer_dockspace_flags);

		if (!initialized)
		{
			InitResourceExplorerDockspace();
		}

		if (ImGui::Begin("Resources Folder Explorer"))
		{
			ShowFileSystemActionsMenu(selected_folder);
			ShowFoldersHierarchy(*App->filesystem->assets_file);
		}
		ImGui::End();

		ImGui::SameLine();
		if (ImGui::Begin("Resources File Explorer")) {
			ShowFileSystemActionsMenu(selected_file);
			ShowFilesInExplorer();
		}
		ImGui::End();
	}
	ImGui::End();
}

void PanelResourcesExplorer::InitResourceExplorerDockspace()
{
	ImGui::DockBuilderRemoveNode(resources_explorer_dockspace_id); // Clear out existing layout
	ImGui::DockBuilderAddNode(resources_explorer_dockspace_id, ImGuiDockNodeFlags_DockSpace); // Add empty node
	ImVec2 child_window_size = ImGui::GetWindowSize();
	ImGui::DockBuilderSetNodeSize(resources_explorer_dockspace_id, child_window_size);

	ImGuiID dock_main_id = resources_explorer_dockspace_id; // This variable will track the document node, however we are not using it here as we aren't docking anything into it.

	ImGuiID dock_id_left;
	ImGuiID dock_id_right = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.80f, NULL, &dock_id_left);


	ImGui::DockBuilderDockWindow("Resources Folder Explorer", dock_id_left);
	ImGui::DockBuilderDockWindow("Resources File Explorer", dock_id_right);
	
	ImGuiDockNode* left_node = ImGui::DockBuilderGetNode(dock_id_left);
	left_node->LocalFlags = ImGuiDockNodeFlags_NoTabBar;

	ImGuiDockNode* right_node = ImGui::DockBuilderGetNode(dock_id_right);
	right_node->LocalFlags |= ImGuiDockNodeFlags_NoTabBar;

	ImGui::DockBuilderFinish(resources_explorer_dockspace_id);
}

void PanelResourcesExplorer::ShowFoldersHierarchy(const File & file)
{
	for (auto & child : file.children)
	{
		if (child->file_type == FileType::DIRECTORY)
		{

			ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen;

			std::string filename = ICON_FA_FOLDER " " + child->filename;
			if (child->sub_folders == 0)
			{
				flags |= ImGuiTreeNodeFlags_Leaf;
			}
			if (selected_folder == child.get())
			{
				flags |= ImGuiTreeNodeFlags_Selected;
				filename = ICON_FA_FOLDER_OPEN " " + child->filename;
			}
			bool expanded = ImGui::TreeNodeEx(filename.c_str(), flags);
			if (expanded) {
				ImGui::PushID(filename.c_str());
				ProcessMouseInput(child.get(), true);
				ShowFoldersHierarchy(*child);
				ImGui::PopID();
				ImGui::TreePop();
			}
		}
	}
}

void PanelResourcesExplorer::ShowFilesInExplorer()
{

	if (selected_folder == nullptr)
	{
		return;
	}
	ImGuiStyle& style = ImGui::GetStyle();
	size_t files_count = 0;

	float window_visible_x2 = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;
	for (auto & file : selected_folder->children)
	{
			std::string item_name;
			std::string filename = std::string(file->filename);
			std::string spaces;
			for (size_t i = 0; i < (filename.size()/2); i++)
			{
				spaces += " ";
			}
			if (file->file_type == FileType::DIRECTORY) {
				item_name = spaces + std::string(ICON_FA_FOLDER "\n " + filename);
			}
			else if(file->file_type == FileType::ARCHIVE)
			{
				item_name = spaces + std::string(ICON_FA_ARCHIVE "\n " + filename);
			}
			else if (file->file_type == FileType::TEXTURE)
			{
				item_name = spaces + std::string(ICON_FA_IMAGE "\n " + filename);
			}
			else if (file->file_type == FileType::MODEL)
			{
				item_name = spaces + std::string(ICON_FA_CUBES "\n " + filename);
			}
			else {
				item_name = spaces + std::string(ICON_FA_BOX "\n " + filename);
			}
			ImVec2 text_sz(ImGui::CalcTextSize(filename.c_str()).x+5,0);
			ImGui::Selectable(item_name.c_str(), selected_file == file.get(),ImGuiSelectableFlags_None,text_sz);
			ProcessMouseInput(file.get(), false);
			FilesDrag();
			++files_count;
			float last_button_x2 = ImGui::GetItemRectMax().x;
			float next_button_x2 = last_button_x2 + style.ItemSpacing.x + text_sz.x; // Expected position if next text was on same line
			if (files_count + 1 < 50 && next_button_x2 < window_visible_x2)
				ImGui::SameLine();
	}
}

void PanelResourcesExplorer::ProcessMouseInput(File * file, bool in_folders_windows)
{
	if (ImGui::IsItemHovered())
	{
		if (ImGui::IsMouseClicked(0) && in_folders_windows)
		{
			selected_folder = file;
			selected_file = nullptr;
		}
		else if (ImGui::IsMouseDoubleClicked(0) && file->file_type == FileType::DIRECTORY)
		{
			selected_folder = file;
		}
		else if(ImGui::IsMouseClicked(0) )
		{
			selected_file = file;
		}
	}
}

void PanelResourcesExplorer::ShowFileSystemActionsMenu(const File * file)
{
	std::string label("Menu");
	bool changes = false;
	if (ImGui::BeginPopupContextWindow(label.c_str()))
	{
	
		if (ImGui::BeginMenu("Create"))
		{
			if (ImGui::Selectable("Folder"))
			{
				MakeDirectoryFromFile(selected_folder);
				changes = true;
			}
			ImGui::EndMenu();
		}
		if (ImGui::Selectable("Delete"))
		{
			bool success = App->filesystem->Remove(file);
			if (success)
			{
				selected_file = nullptr;
				changes = true;
			}
		}
		if (changes)
		{
			App->filesystem->RefreshFilesHierarchy();
		}
		/*if (ImGui::Selectable("Rename"))
		{
			//TODO
		}
		if (ImGui::Selectable("Copy"))
		{
			//TODO
		}*/

		ImGui::EndPopup();
	}
}

void PanelResourcesExplorer::MakeDirectoryFromFile(File * file)
{
	if (file == nullptr)
	{
		return;
	}
	std::shared_ptr<File> new_folder;
	if (!file->file_path.empty() && file->file_type != FileType::DIRECTORY)
	{
		size_t last_slash = file->file_path.find_last_of("/");
		new_folder = std::make_shared<File>(App->filesystem->MakeDirectory(file->file_path.substr(0, last_slash - 1)+"/new Folder"));
	}
	else if(!file->file_path.empty())
	{
		new_folder = std::make_shared<File>(App->filesystem->MakeDirectory(file->file_path+"/new Folder"));
	}
	else if(!selected_folder->file_path.empty()){
		new_folder = std::make_shared<File>(App->filesystem->MakeDirectory(selected_folder->file_path+ "/new Folder"));
	}
	file->children.push_back(new_folder);
	selected_folder = new_folder.get();
}

void PanelResourcesExplorer::CopyFileToSelectedFolder(const char * source) const
{
	std::string source_string(source);
	std::replace(source_string.begin(), source_string.end(), '\\', '/');
	std::string file_name = source_string.substr(source_string.find_last_of('/'), -1);
	std::string destination;
	if (selected_folder == nullptr)
	{
		destination = "Assets"+ file_name;
	}
	else
	{
		destination = selected_folder->file_path  + file_name;
	}
	App->filesystem->Copy(source, destination.c_str());
}

void PanelResourcesExplorer::FilesDrag() const
{
	if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
	{
		ImGui::SetDragDropPayload("DND_File", &selected_file, sizeof(File*));
		ImGui::Text("Dragging %s", selected_file->filename.c_str());
		ImGui::EndDragDropSource();
	}
}