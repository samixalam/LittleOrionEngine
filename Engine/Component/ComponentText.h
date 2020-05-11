#ifndef _COMPONENTTEXT_H_
#define _COMPONENTTEXT_H_

#include "Component.h"

#include <string>

class Font;

class ComponentText : public Component
{
public:
	ComponentText();
	ComponentText(GameObject * owner);
	~ComponentText() = default;

	Component* Clone(bool original_prefab = false) const override;
	void Copy(Component* component_to_copy) const override;

	void Delete() override;
	void SpecializedSave(Config& config) const override;
	void SpecializedLoad(const Config& config) override;

	void Render(float4x4* projection);

	void SetFont(uint32_t font_uuid);

private:
	void InitData();

public:
	//Text Inputs
	std::string text = "Default";
	float scale = 12.0f;
	float text_width = 0;
	float text_heigth = 0;

public:
	uint32_t font_uuid = 0;
	std::shared_ptr<Font> font = nullptr;

};
#endif