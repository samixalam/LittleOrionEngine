#include "Globals.h"
#include "Application.h"
#include "ModuleInput.h"
#include "ModuleWindow.h"
#include "ModuleModelLoader.h"
#include "ModuleCamera.h"
#include "ModuleRender.h"
#include "ModuleScene.h"

#include <SDL/SDL.h>
#include "imgui.h"
#include "imgui_impl_sdl.h"
#include <FontAwesome5/IconsFontAwesome5.h>
#include <GL/glew.h>

// Called before render is available
bool ModuleInput::Init()
{
	APP_LOG_SECTION("************ Module Input Init ************");

	APP_LOG_INIT("Init SDL input event system");
	bool ret = true;
	SDL_Init(0);

	if(SDL_InitSubSystem(SDL_INIT_EVENTS) < 0)
	{
		APP_LOG_ERROR("SDL_EVENTS could not initialize! SDL_Error: %s\n", SDL_GetError());
		ret = false;
	}

	APP_LOG_SUCCESS("SDL input event system initialized correctly.");

	return ret;
}

// Called every draw update
update_status ModuleInput::PreUpdate()
{
	SDL_PumpEvents();

	SDL_Event event;
	while (SDL_PollEvent(&event) != 0)
	{
		ImGui_ImplSDL2_ProcessEvent(&event);

		// Esc button is pressed
		switch (event.type)
		{
		case SDL_QUIT:
			return update_status::UPDATE_STOP;
			break;

		case SDL_WINDOWEVENT:
			if (event.window.event == SDL_WINDOWEVENT_RESIZED || event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
				App->window->WindowResized(event.window.data1, event.window.data2);
			break;

		case SDL_MOUSEMOTION:
			if (event.motion.state & SDL_BUTTON_RMASK && App->scene->scene_window_is_hovered) {
				if (math::Abs(event.motion.xrel) > 1.5) {
					App->cameras->RotateYaw(event.motion.xrel);
				}

				if (math::Abs(event.motion.yrel) > 1.5) {
					App->cameras->RotatePitch(event.motion.yrel);
				}

			}
			else if (event.motion.state & SDL_BUTTON_LMASK && App->scene->scene_window_is_hovered) {
				if (math::Abs(event.motion.xrel) > 1.5) {
					App->cameras->OrbitX(event.motion.xrel);
				}

				if (math::Abs(event.motion.yrel) > 1.5) {
					App->cameras->OrbitY(event.motion.yrel);
				}

			}
			break;

		case SDL_MOUSEWHEEL:
			if (event.wheel.y > 0 && App->scene->scene_window_is_hovered)
			{
				App->cameras->MoveFoward();
			}
			else if (event.wheel.y < 0 && App->scene->scene_window_is_hovered)
			{
				App->cameras->MoveBackward();
			}
			break;

		case SDL_MOUSEBUTTONDOWN:
			if (event.button.button == SDL_BUTTON_RIGHT && App->scene->scene_window_is_hovered)
			{
				App->cameras->SetMovement(true);
			}
			break;

		case SDL_MOUSEBUTTONUP:
			if (event.button.button == SDL_BUTTON_RIGHT)
			{
				App->cameras->SetMovement(false);
			}
			break;

		case SDL_KEYDOWN:
			if (event.key.keysym.sym == SDLK_LALT)
			{
				App->cameras->SetOrbit(true);
			}
			else if (event.key.keysym.sym == SDLK_LSHIFT)
			{
				App->cameras->SetSpeedUp(true);
			}
			else if (event.key.keysym.sym == SDLK_f)
			{
				//App->cameras->Focus(*App->model_loader->current_model->bounding_box); TODO THIS
			}
			else if (event.key.keysym.sym == SDLK_b)
			{
				App->renderer->bounding_box_visible = !App->renderer->bounding_box_visible;
			}
			break;

		case SDL_KEYUP:
			if (event.key.keysym.sym == SDLK_LALT)
			{
				App->cameras->SetOrbit(false);
			}
			else if (event.key.keysym.sym == SDLK_LSHIFT)
			{
				App->cameras->SetSpeedUp(false);
			}
			break;

		case SDL_DROPFILE:
			char *dropped_filedir = event.drop.file;
			switch (GetFileType(dropped_filedir))
			{
			case FileType::MODEL:
				App->model_loader->LoadModel(dropped_filedir);
				break;
			default:
				break;
			}
			SDL_free(dropped_filedir);
			
			break;
		}
	}

	keyboard = SDL_GetKeyboardState(NULL);

	if (App->cameras->MovementEnabled())
	{
		if (keyboard[SDL_SCANCODE_Q]) 
		{
			App->cameras->MoveUp();
		}

		if (keyboard[SDL_SCANCODE_E])
		{
			App->cameras->MoveDown();
		}

		if (keyboard[SDL_SCANCODE_W])
		{
			App->cameras->MoveFoward();
		}

		if (keyboard[SDL_SCANCODE_S])
		{
			App->cameras->MoveBackward();
		}

		if (keyboard[SDL_SCANCODE_A])
		{
			App->cameras->MoveLeft();
		}

		if (keyboard[SDL_SCANCODE_D])
		{
			App->cameras->MoveRight();
		}
	}

	if (keyboard[SDL_SCANCODE_UP])
	{
		App->cameras->RotatePitch(-1.f);
	}

	if (keyboard[SDL_SCANCODE_DOWN])
	{
		App->cameras->RotatePitch(1.f);
	}

	if (keyboard[SDL_SCANCODE_LEFT])
	{
		App->cameras->RotateYaw(-1.f);
	}

	if (keyboard[SDL_SCANCODE_RIGHT])
	{
		App->cameras->RotateYaw(1.f);
	}
	return update_status::UPDATE_CONTINUE;
}

// Called before quitting
bool ModuleInput::CleanUp()
{
	APP_LOG_INFO("Quitting SDL input event subsystem");
	SDL_QuitSubSystem(SDL_INIT_EVENTS);
	return true;
}

ModuleInput::FileType ModuleInput::GetFileType(const char *file_path) const
{
	std::string file_extension = GetFileExtension(file_path);

	if (
		file_extension == "png"
		|| file_extension == "PNG"
		|| file_extension == "dds"
		|| file_extension == "DDS"
	)
	{
		return ModuleInput::FileType::TEXTURE;
	}
	if (
		file_extension == "fbx"
		|| file_extension == "FBX"
	)
	{
		return ModuleInput::FileType::MODEL;
	}

	return ModuleInput::FileType::UNKNOWN;
}

std::string ModuleInput::GetFileExtension(const char *file_path) const
{
	std::string file_path_string = std::string(file_path);
	std::size_t found = file_path_string.find_last_of(".");

	std::string file_extension = file_path_string.substr(found + 1, file_path_string.length());

	return file_extension;
}


void ModuleInput::ShowInputOptions()
{
	if (ImGui::CollapsingHeader(ICON_FA_KEYBOARD " Input")) 
	{
		ImGuiIO& io = ImGui::GetIO();

		// Display ImGuiIO output flags
		ImGui::Text("WantCaptureMouse: %d", io.WantCaptureMouse);
		ImGui::Text("WantCaptureKeyboard: %d", io.WantCaptureKeyboard);
		ImGui::Text("WantTextInput: %d", io.WantTextInput);
		ImGui::Text("WantSetMousePos: %d", io.WantSetMousePos);
		ImGui::Text("NavActive: %d, NavVisible: %d", io.NavActive, io.NavVisible);

		// Display Keyboard/Mouse state
		if (ImGui::TreeNode("Keyboard, Mouse & Navigation State"))
		{
			if (ImGui::IsMousePosValid())
				ImGui::Text("Mouse pos: (%g, %g)", io.MousePos.x, io.MousePos.y);
			else
				ImGui::Text("Mouse pos: <INVALID>");
			ImGui::Text("Mouse delta: (%g, %g)", io.MouseDelta.x, io.MouseDelta.y);
			ImGui::Text("Mouse down:");     for (int i = 0; i < IM_ARRAYSIZE(io.MouseDown); i++) if (io.MouseDownDuration[i] >= 0.0f) { ImGui::SameLine(); ImGui::Text("b%d (%.02f secs)", i, io.MouseDownDuration[i]); }
			ImGui::Text("Mouse clicked:");  for (int i = 0; i < IM_ARRAYSIZE(io.MouseDown); i++) if (ImGui::IsMouseClicked(i)) { ImGui::SameLine(); ImGui::Text("b%d", i); }
			ImGui::Text("Mouse dbl-clicked:"); for (int i = 0; i < IM_ARRAYSIZE(io.MouseDown); i++) if (ImGui::IsMouseDoubleClicked(i)) { ImGui::SameLine(); ImGui::Text("b%d", i); }
			ImGui::Text("Mouse released:"); for (int i = 0; i < IM_ARRAYSIZE(io.MouseDown); i++) if (ImGui::IsMouseReleased(i)) { ImGui::SameLine(); ImGui::Text("b%d", i); }
			ImGui::Text("Mouse wheel: %.1f", io.MouseWheel);

			ImGui::Text("Keys down:");      for (int i = 0; i < IM_ARRAYSIZE(io.KeysDown); i++) if (io.KeysDownDuration[i] >= 0.0f) { ImGui::SameLine(); ImGui::Text("%d (0x%X) (%.02f secs)", i, i, io.KeysDownDuration[i]); }
			ImGui::Text("Keys pressed:");   for (int i = 0; i < IM_ARRAYSIZE(io.KeysDown); i++) if (ImGui::IsKeyPressed(i)) { ImGui::SameLine(); ImGui::Text("%d (0x%X)", i, i); }
			ImGui::Text("Keys release:");   for (int i = 0; i < IM_ARRAYSIZE(io.KeysDown); i++) if (ImGui::IsKeyReleased(i)) { ImGui::SameLine(); ImGui::Text("%d (0x%X)", i, i); }
			ImGui::Text("Keys mods: %s%s%s%s", io.KeyCtrl ? "CTRL " : "", io.KeyShift ? "SHIFT " : "", io.KeyAlt ? "ALT " : "", io.KeySuper ? "SUPER " : "");
			ImGui::Text("Chars queue:");    for (int i = 0; i < io.InputQueueCharacters.Size; i++) { ImWchar c = io.InputQueueCharacters[i]; ImGui::SameLine();  ImGui::Text("\'%c\' (0x%04X)", (c > ' ' && c <= 255) ? (char)c : '?', c); } // FIXME: We should convert 'c' to UTF-8 here but the functions are not public.

			ImGui::Text("NavInputs down:"); for (int i = 0; i < IM_ARRAYSIZE(io.NavInputs); i++) if (io.NavInputs[i] > 0.0f) { ImGui::SameLine(); ImGui::Text("[%d] %.2f", i, io.NavInputs[i]); }
			ImGui::Text("NavInputs pressed:"); for (int i = 0; i < IM_ARRAYSIZE(io.NavInputs); i++) if (io.NavInputsDownDuration[i] == 0.0f) { ImGui::SameLine(); ImGui::Text("[%d]", i); }
			ImGui::Text("NavInputs duration:"); for (int i = 0; i < IM_ARRAYSIZE(io.NavInputs); i++) if (io.NavInputsDownDuration[i] >= 0.0f) { ImGui::SameLine(); ImGui::Text("[%d] %.2f", i, io.NavInputsDownDuration[i]); }


			ImGui::TreePop();
		}


		if (ImGui::TreeNode("Dragging"))
		{
			for (int button = 0; button < 3; button++)
				ImGui::Text("IsMouseDragging(%d):\n  w/ default threshold: %d,\n  w/ zero threshold: %d\n  w/ large threshold: %d",
					button, ImGui::IsMouseDragging(button), ImGui::IsMouseDragging(button, 0.0f), ImGui::IsMouseDragging(button, 20.0f));

			ImVec2 value_raw = ImGui::GetMouseDragDelta(0, 0.0f);
			ImVec2 value_with_lock_threshold = ImGui::GetMouseDragDelta(0);
			ImVec2 mouse_delta = io.MouseDelta;
			ImGui::Text("GetMouseDragDelta(0):\n  w/ default threshold: (%.1f, %.1f),\n  w/ zero threshold: (%.1f, %.1f)\nMouseDelta: (%.1f, %.1f)", value_with_lock_threshold.x, value_with_lock_threshold.y, value_raw.x, value_raw.y, mouse_delta.x, mouse_delta.y);
			ImGui::TreePop();
		}
	}
}