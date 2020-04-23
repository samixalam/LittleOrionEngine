#include "ModuleScriptManager.h"

#include "Component/ComponentScript.h"
#include "DLL/DLLManager.h"
#include "Filesystem/Path.h"
#include "Filesystem/PathAtlas.h"
#include "Helper/Utils.h"
#include "Main/Application.h"
#include "Main/GameObject.h"
#include "Module/ModuleFileSystem.h"
#include "Module/ModuleTime.h"

#include "Script/Script.h"

#include <algorithm>


bool ModuleScriptManager::Init()
{
	APP_LOG_SECTION("************ Module Manager Script ************");

	dll = new DLLManager();

	LoadScriptList();

	return true;
}

update_status ModuleScriptManager::Update()
{
	
	if (!scripts.empty() && App->time->isGameRunning()) 
	{
		RunScripts();
	}
	if (!App->time->isGameRunning()) 
	{
		if(dll->DLLItsUpdated())
		{
			ReloadDLL();
		}
	}
	return update_status::UPDATE_CONTINUE;
}

bool ModuleScriptManager::CleanUp()
{
	
	dll->CleanUp();
	delete dll;

	return true;
}

ComponentScript* ModuleScriptManager::CreateComponentScript()
{
	ComponentScript* new_script = new ComponentScript();
	scripts.push_back(new_script);
	return new_script;
}

void ModuleScriptManager::RemoveComponentScript(ComponentScript* script_to_remove)
{
	const auto it = std::find(scripts.begin(), scripts.end(), script_to_remove);
	if (it != scripts.end())
	{
		delete *it;
		scripts.erase(it);
	}
}

void ModuleScriptManager::CreateScript(const std::string& name)
{
	FileData cpp_file_data = App->filesystem->GetPath(RESOURCES_SCRIPT_TEMPLATE_CPP)->GetFile()->Load();
	std::string cpp_file((char*)cpp_file_data.buffer, cpp_file_data.size);
	free((char*)cpp_file_data.buffer);

	FileData header_file_data = App->filesystem->GetPath(RESOURCES_SCRIPT_TEMPLATE_H)->GetFile()->Load();
	std::string header_file((char*)header_file_data.buffer, header_file_data.size);
	free((char*)header_file_data.buffer);

	Utils::ReplaceStringInPlace(cpp_file, "TemplateScript", name);
	Utils::ReplaceStringInPlace(header_file, "TemplateScript", name);

	std::string name_uppercase = name;
	std::transform(name_uppercase.begin(), name_uppercase.end(), name_uppercase.begin(), ::toupper);
	Utils::ReplaceStringInPlace(header_file, "_TEMPLATESCRIPT_H_", "_" + name_uppercase + "_H_");

	if (!App->filesystem->Exists((SCRIPT_PATH + name + ".cpp").c_str()))
	{
		//TODO: Use filesystem for this
		Utils::SaveFileContent(cpp_file, SCRIPT_PATH + name + ".cpp");
		Utils::SaveFileContent(header_file, SCRIPT_PATH + name + ".h");
		scripts_list.push_back(name);
	}
	
}

void ModuleScriptManager::InitResourceScript()
{
	if (dll->gameplay_dll != nullptr)
	{
		for (const auto& component_script : scripts)
		{
			CREATE_SCRIPT script_func = (CREATE_SCRIPT)GetProcAddress(dll->gameplay_dll, (component_script->name + "DLL").c_str());
			if (script_func != nullptr)
			{
				delete component_script->script;
				component_script->script = script_func();
				component_script->script->AddReferences(component_script->owner, App);
				component_script->script->InitPublicGameObjects();
			}
		}
	}
}

Script* ModuleScriptManager::CreateResourceScript(const std::string& script_name, GameObject* owner) 
{
	if (dll->gameplay_dll != nullptr)
	{
		CREATE_SCRIPT script_func = (CREATE_SCRIPT)GetProcAddress(dll->gameplay_dll, (script_name+"DLL").c_str());
		if (script_func != nullptr)
		{
			Script* script = script_func();
			script->AddReferences(owner, App);
			script->InitPublicGameObjects();
			return script;
		}
	}
	return nullptr;
}

void ModuleScriptManager::LoadScriptList() 
{
	if (scripts_list.size() > 0) 
	{
		scripts_list.clear();
	}
	Path* scripts_list_file_path = App->filesystem->GetPath("/Assets/Scripts/src/Script");

	for(const auto& script : scripts_list_file_path->children )
	{
		std::string script_to_search = script->GetFilenameWithoutExtension();
		bool found = std::find(scripts_list.begin(), scripts_list.end(), script_to_search) != scripts_list.end();
		if(!found)
		{
			scripts_list.emplace_back(script_to_search);
		}
	}

}

void ModuleScriptManager::InitScripts()
{
	for (const auto& component_script : scripts)
	{
		component_script->AwakeScript();
	}
	for (const auto& component_script : scripts)
	{
		component_script->StartScript();
	}
}

void ModuleScriptManager::RunScripts()
{
	for (const auto& component_script : scripts)
	{
		component_script->Update();
	}
}

void ModuleScriptManager::RemoveScriptPointers()
{
	for (const auto& component_script : scripts)
	{
		component_script->script = nullptr;
	}
}

void ModuleScriptManager::ReloadDLL() 
{
	std::unordered_map<uint64_t, Config> config_list;
	SaveVariables(config_list);
	if (dll->gameplay_dll != nullptr) 
	{
		dll->ReloadDLL();
		RemoveScriptPointers();
		dll->InitDLL();
	}
	InitResourceScript();
	LoadVariables(config_list);
}

void ModuleScriptManager::Refresh()
{
	LoadScriptList();
	ReloadDLL();
}

void ModuleScriptManager::ReLink()
{
	for (const auto& component_script : scripts)
	{
		component_script->script->Link();
	}
}

void ModuleScriptManager::SaveVariables(std::unordered_map<uint64_t, Config>& config_list)
{
	for (const auto& component_script : scripts)
	{
		if (component_script->script != nullptr) 
		{
			Config config;
			component_script->script->Save(config);
			config_list.insert({ component_script->UUID, config });
		}

	}
}

void ModuleScriptManager::LoadVariables(std::unordered_map<uint64_t, Config> config_list)
{
	for (const auto& component_script : scripts)
	{
		if (component_script->script != nullptr)
		{
			std::unordered_map<uint64_t, Config>::const_iterator got = config_list.find(component_script->UUID);
			if (got != config_list.end()) {
				component_script->script->Load(got->second);
				component_script->script->Link();
			}
		}
		
	}
}