#include "Globals.h"
#include "Application.h"
#include "ModuleProgram.h"
#include "SDL.h"

ModuleProgram::ModuleProgram()
{
}

// Destructor
ModuleProgram::~ModuleProgram()
{
}

// Called before render is available
bool ModuleProgram::Init()
{
	loadProgram(default_program, DEFAULT_VERTEX_SHADER_PATH, DEFAULT_FRAGMENT_SHADER_PATH);
	loadProgram(texture_program, DEFAULT_VERTEX_SHADER_PATH, TEXTURE_FRAGMENT_SHADER_PATH); // TODO: Exit when error in loading

	return true;
}

update_status ModuleProgram::PreUpdate()
{
	return UPDATE_CONTINUE;
}

// Called every draw update
update_status ModuleProgram::Update()
{

	return UPDATE_CONTINUE;
}

update_status ModuleProgram::PostUpdate()
{
	return UPDATE_CONTINUE;
}

// Called before quitting
bool ModuleProgram::CleanUp()
{
	glDeleteProgram(default_program);
	glDeleteProgram(texture_program);
	return true;
}

bool ModuleProgram::loadProgram(GLuint &shader_program, const char* vertex_shader_file_name, const char* fragment_shader_file_name)
{
	GLuint vertex_shader;
	GLuint fragment_shader;

	if (!initVertexShader(vertex_shader, vertex_shader_file_name))
	{
		return false;
	}

	if (!initFragmentShader(fragment_shader, fragment_shader_file_name))
	{
		return false;
	}

	if (!initProgram(shader_program, vertex_shader, fragment_shader))
	{
		return false;
	}
}

bool ModuleProgram::initVertexShader(GLuint &vertex_shader, const char* vertex_shader_file_name)
{
	LOG("Loading vertex shader");
	const char *vertex_shader_loaded_file = loadFile(vertex_shader_file_name);
	vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	if (vertex_shader == 0) {
		LOG("Error creating vertex shader");
		return false;
	}
	glShaderSource(vertex_shader, 1, &vertex_shader_loaded_file, NULL);
	delete vertex_shader_loaded_file;

	LOG("Compiling vertex shader");
	glCompileShader(vertex_shader);
	int compilation_status;
	char info_log[512];
	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &compilation_status);
	if (!compilation_status)
	{
		glGetShaderInfoLog(vertex_shader, 512, NULL, info_log);
		LOG("Error compiling vertex shader");
		LOG(info_log);
		return false;
	}

	return true;
}

bool ModuleProgram::initFragmentShader(GLuint &fragment_shader, const char* fragment_shader_file_name)
{
	LOG("Loading fragment shader");
	const char *fragment_shader_loaded_file = loadFile(fragment_shader_file_name);
	fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	if (fragment_shader == 0) {
		LOG("Error creating fragment shader");
		return false;
	}
	glShaderSource(fragment_shader, 1, &fragment_shader_loaded_file, NULL);
	delete fragment_shader_loaded_file;

	LOG("Compiling fragment shader");
	glCompileShader(fragment_shader);
	int compilation_status;
	char info_log[512];
	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &compilation_status);
	if (!compilation_status)
	{
		glGetShaderInfoLog(fragment_shader, 512, NULL, info_log);
		LOG("Error compiling fragment shader");
		LOG(info_log);
		return false;
	}

	return true;
}

bool ModuleProgram::initProgram(GLuint &shader_program, const GLuint vertex_shader, const GLuint fragment_shader)
{
	LOG("Creating shader program");
	shader_program = glCreateProgram();
	if (shader_program == 0) {
		LOG("Error creating shader program");
		return false;
	}

	LOG("Attaching vertex shader to shader program");
	glAttachShader(shader_program, vertex_shader);

	LOG("Attaching fragment shader to shader program");
	glAttachShader(shader_program, fragment_shader);

	LOG("Linking shader program");
	glLinkProgram(shader_program);

	LOG("Deleting vertex shader");
	glDeleteShader(vertex_shader);

	LOG("Deleting fragment shader");
	glDeleteShader(fragment_shader);

	int compilation_status;
	char info_log[512];
	glGetProgramiv(shader_program, GL_LINK_STATUS, &compilation_status);
	if (!compilation_status) {
		glGetProgramInfoLog(shader_program, 512, NULL, info_log);
		LOG("Error linking shader program");
		LOG(info_log);
		return false;
	}

	return true;
}

const char* ModuleProgram::loadFile(const char* file_name)
{
	char* data = nullptr;
	FILE* file = nullptr;
	fopen_s(&file, file_name, "rb");
	if (file)
	{
		fseek(file, 0, SEEK_END);
		int size = ftell(file);
		rewind(file);
		data = (char*)malloc(size + 1);
		fread(data, 1, size, file);
		data[size] = 0;
		fclose(file);
	}

	return data;
}