#ifndef _COMPONENTLIGHT_H
#define _COMPONENTLIGHT_H

#include "Component.h"

class GameObject;
class ComponentLight : public Component
{
public:
	ComponentLight();
	ComponentLight(GameObject * owner);
	~ComponentLight() = default;
	

	void Delete() override;
	void Render(unsigned int shader_program) const override;

	void Save(Config& config) const override;
	void Load(const Config &config) override;


	void ShowComponentWindow() override;

public:
	float light_color[3] = { 1.0f, 1.0f, 1.0f};
	float light_intensity = 0.1f; 
};

#endif // !_COMPONENTLIGHT_H
