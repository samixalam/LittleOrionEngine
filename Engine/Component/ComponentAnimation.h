#ifndef _COMPONENTANIMATION_H_
#define _COMPONENTANIMATION_H_

#include "Component.h"

#include "EditorUI/Panel/InspectorSubpanel/PanelComponent.h"

#include <GL/glew.h>
#include <memory>
class Animation;
class AnimController;
class GameObject;
class Skeleton;

class ComponentAnimation :	public Component
{
public:
	ComponentAnimation();
	ComponentAnimation(GameObject* owner);
	~ComponentAnimation();


	//Copy and move
	ComponentAnimation(const ComponentAnimation& component_to_copy) = default;
	ComponentAnimation(ComponentAnimation&& component_to_move) = default;

	ComponentAnimation & operator=(const ComponentAnimation& component_to_copy) = default;
	ComponentAnimation & operator=(ComponentAnimation&& component_to_move) = default;

	Component* Clone(bool original_prefab = false) const override;
	void Copy(Component* component_to_copy) const override;

	void SetAnimation(std::shared_ptr<Animation> & animation);
	void SetSkeleton(std::shared_ptr<Skeleton> & skeleton);

	void Render(GLuint shader_program);

	void Update() override;
	void Delete() override;

	void Save(Config& config) const override;
	void Load(const Config& config) override;

	void UpdateBone(GameObject* bone);

public:
	AnimController* animation_controller = nullptr;

private:
	friend class PanelComponent;
	std::vector<float4x4> palette;
};

#endif //_COMPONENTANIMATION_H_

