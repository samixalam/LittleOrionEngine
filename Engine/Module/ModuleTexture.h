#ifndef _MODULETEXTURE_H_
#define _MODULETEXTURE_H_

#include "Module.h"
#include "Globals.h"

#include <GL/glew.h>

struct SDL_Texture;
struct SDL_Renderer;
struct SDL_Rect;

class ComponentMaterial;
class Texture;

class ModuleTexture : public Module
{
public:
	ModuleTexture() = default;
	~ModuleTexture() = default;

	bool Init();
	bool CleanUp();

	ComponentMaterial* CreateComponentMaterial() const;
	
	Texture* LoadTexture(const char* texture_path) const;
	GLuint  LoadCubemap(std::vector<std::string> faces) const;
	unsigned char* LoadImageData(const char* texture_path, int& width, int& height) const;

	void GenerateCheckerboardTexture();

public:
	GLuint checkerboard_texture_id;
};

#endif //_MODULETEXTURE_H_
