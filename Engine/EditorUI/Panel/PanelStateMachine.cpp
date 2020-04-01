#include "PanelStateMachine.h"
#include "Filesystem/File.h"
#include <FontAwesome5/IconsFontAwesome5.h>

#include "Main/Application.h"
#include "Module/ModuleResourceManager.h"
#include "imgui_internal.h"

PanelStateMachine::PanelStateMachine()
{
	opened = false;
	enabled = true;
	window_name = ICON_FA_PROJECT_DIAGRAM " State Machine";
	editor_context = ax::NodeEditor::CreateEditor();
}

PanelStateMachine::~PanelStateMachine()
{
	nodes.erase(nodes.begin(), nodes.end());
	links.erase(links.begin(), links.end());
	ax::NodeEditor::DestroyEditor(editor_context);
}

void PanelStateMachine::Render()
{
	if (ImGui::Begin(ICON_FA_PROJECT_DIAGRAM " State Machine", &opened, ImGuiWindowFlags_MenuBar))
	{
			ax::NodeEditor::SetCurrentEditor(editor_context);
			{
				LeftPanel();
				ax::NodeEditor::Begin("My Editor");
				if (ImGui::IsMouseClicked(1))
				{
					ax::NodeEditor::Suspend();
					ImGui::OpenPopup("Editor Menu");
					ax::NodeEditor::Resume();
				}
				RenderStates();
				HandleInteraction();
				CreateNodeMenu();
			}
			ax::NodeEditor::End();
			if (firstFrame)
			{
				ax::NodeEditor::NavigateToContent(0.0f);
				firstFrame = false;
			}
			ImGuiContext* context = ImGui::GetCurrentContext();

	}
	ImGui::End();
}
void PanelStateMachine::RenderStates() const
{
	ImVec2 position(10,10);
	for (auto & node : nodes)
	{
		assert(!node->id.Invalid);
		// Start drawing nodes.
		if (firstFrame)
		{
			ax::NodeEditor::SetNodePosition(node->id, position);
		}
		ax::NodeEditor::BeginNode(node->id);
		ImGui::Text(node->state->name.c_str());
		ImGui::Button("Clip");
		for (auto & output :  node->outputs)
		{
			ax::NodeEditor::BeginPin(output, ax::NodeEditor::PinKind::Output);
			ImGui::Text("Out ->");
			ax::NodeEditor::EndPin();
		}
		for (auto & inputs : node->inputs)
		{
			ax::NodeEditor::BeginPin(inputs, ax::NodeEditor::PinKind::Input);
			ImGui::Text("-> In");
			ax::NodeEditor::EndPin();
		}
		ax::NodeEditor::EndNode();

		position = ax::NodeEditor::GetNodePosition(node->id);
		position.x+= ax::NodeEditor::GetNodeSize(node->id).x;

	}
	for (auto& linkInfo : links)
	{
		ax::NodeEditor::Link(linkInfo->id, linkInfo->input_id, linkInfo->output_id);
	}
}

void PanelStateMachine::HandleInteraction()
{
	if (ax::NodeEditor::BeginCreate())
	{
		ax::NodeEditor::PinId input_pin_id, output_pin_id;
		if (ax::NodeEditor::QueryNewLink(&input_pin_id, &output_pin_id))
		{
			if (input_pin_id && output_pin_id)
			{
				if (ax::NodeEditor::AcceptNewItem())
				{
					// Since we accepted new link, lets add one to our list of links.
					std::shared_ptr<Transition> new_transition = std::make_shared<Transition>();
					for (auto node : nodes)
					{
						auto& it_input = std::find(node->inputs.begin(), node->inputs.end(), input_pin_id);
						if (it_input != node->inputs.end())
						{
							new_transition->source_hash = node->state->name_hash;
						}
						auto& it_output = std::find(node->outputs.begin(), node->outputs.end(), output_pin_id);
						if (it_output != node->outputs.end())
						{
							new_transition->target_hash = node->state->name_hash;
						}
					}
					links.push_back(new LinkInfo{ ax::NodeEditor::LinkId(uniqueid++) , input_pin_id, output_pin_id, new_transition });

					// Draw new link.
					ax::NodeEditor::Link(links.back()->id, links.back()->input_id, links.back()->output_id);
				}

			}
		}
	}
	ax::NodeEditor::EndCreate(); // Wraps up object creation action handling.
}
void PanelStateMachine::CreateNodeMenu()
{
	ax::NodeEditor::Suspend();
	if (ImGui::BeginPopup("Editor Menu"))
	{
		if (ImGui::BeginMenu("Create State"))
		{
			if (ImGui::MenuItem("Empty"))
			{
				NodeInfo* node = new NodeInfo({ ax::NodeEditor::NodeId(uniqueid++) });
				node->state = std::make_shared<State>("New Node", nullptr);
				node->inputs.push_back(uniqueid++);
				node->outputs.push_back(uniqueid++);
				nodes.push_back(std::move(node));
			}
			ImGui::EndMenu();
		}
		ImGui::EndPopup();
	}
	ax::NodeEditor::Resume();
}


void PanelStateMachine::LeftPanel()
{
	if (ImGui::BeginChild("Details", ImVec2(300, 0)))
	{
	}
	ImGui::EndChild();
	ImGui::SameLine(0.0f, 12.0f);
}
void PanelStateMachine::OpenStateMachine(const File & file)
{
	nodes.clear();
	links.clear();
	state_machine = std::make_shared<StateMachine>(file.file_path);
	state_machine->Load(file);

	//TODO: Remove, this is only for testing
	ImportOptions options;
	Importer::GetOptionsFromMeta(Importer::GetMetaFilePath(file), options);
	std::shared_ptr<StateMachine> test = App->resources->Load<StateMachine>(options.exported_file);

	//Tranform form state machine to ui
	for (auto & link : state_machine->transitions)
	{
		links.push_back(new LinkInfo{ ax::NodeEditor::LinkId(uniqueid++) ,  ax::NodeEditor::PinId(uniqueid++),  ax::NodeEditor::PinId(uniqueid++), link});
	}
	for (auto & state : state_machine->states)
	{
		NodeInfo * node = new NodeInfo({ ax::NodeEditor::NodeId(uniqueid++) });
		node->state = state;
		for (auto & link : links)
		{
			if (link->transition->source_hash == state->name_hash)
			{
				node->outputs.push_back(link->output_id);
			}
			if (link->transition->target_hash == state->name_hash)
			{
				node->outputs.push_back(link->input_id);
			}
		}
		if (node->state->name_hash == entry_hash && node->outputs.size() == 0)
		{
			node->outputs.push_back(uniqueid++);
		}
		if (node->state->name_hash == end_hash && node->inputs.size() == 0)
		{
			node->inputs.push_back(uniqueid++);
		}
		nodes.push_back(node);
	}
}