#ifndef  __TRIGGERACTIONSCRIPT_H__
#define  __TRIGGERACTIONSCRIPT_H__

#include "Script.h"

class WalkableScript;

class TriggerActionScript : public Script
{
public:
	TriggerActionScript();
	~TriggerActionScript() = default;
	void Awake() override;
	void Start() override;
	void Update() override;
	void OnInspector(ImGuiContext*) override;

	bool OnTriggerEnter();

	GameObject* trigger_go = nullptr;
	WalkableScript* movement_script = nullptr;
	ComponentScript* movement_component = nullptr;
private:
	PanelComponent* panel = nullptr;
	std::string is_object = "None";
};
extern "C" SCRIPT_API TriggerActionScript* TriggerActionScriptDLL(); //This is how we are going to load the script
#endif