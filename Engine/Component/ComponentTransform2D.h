#pragma once
#include "Component.h"
class ComponentTransform2D : public Component
{
public:
	ComponentTransform2D();
	ComponentTransform2D(GameObject * owner);
	ComponentTransform2D(GameObject * owner, const float2 translation, const float rotation, const float2 scale);
	~ComponentTransform2D();

	//Copy and move
	ComponentTransform2D(const ComponentTransform2D& component_to_copy) = default;
	ComponentTransform2D(ComponentTransform2D&& component_to_move) = default;

	ComponentTransform2D & operator=(const ComponentTransform2D & component_to_copy);
	ComponentTransform2D & operator=(ComponentTransform2D && component_to_move) = default;

	float2 position;
	float2 size;
	float rotation;

	// Heredado v�a Component
	virtual void Delete() override;
	virtual Component * Clone(bool create_on_module = true) const override;
	virtual void Copy(Component * component_to_copy) const override;
	virtual void Save(Config & config) const override;
	virtual void Load(const Config & config) override;

private:
	void OnTransformChange();
};

