#include "input.h"

#include <stdio.h>

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include <cimgui.h>

AquaInputManager aqua_input_manager_create(AquaWindow* window) {
	return (AquaInputManager) { .window_ptr = window };
}

bool aqua_input_manager_set_properties(AquaInputManager* input_manager, AquaInputManagerProperties properties) {
	SDL_Window* window = input_manager->window_ptr->sdl_window;

	bool previous = SDL_GetWindowRelativeMouseMode(window);
	if (previous != properties.mouse_captured && !SDL_SetWindowRelativeMouseMode(window, properties.mouse_captured)) {
		fprintf(stderr, "[ERROR] [Aqua] [Input] Failed to set relative mouse mode: %s\n", SDL_GetError());
		return false;
	}

	if (SDL_GetWindowMouseGrab(window) != properties.mouse_captured && !SDL_SetWindowMouseGrab(window, properties.mouse_captured)) {
		fprintf(stderr, "[ERROR] [Aqua] [Input] Failed to set mouse grab: %s\n", SDL_GetError());
		if (!SDL_SetWindowRelativeMouseMode(window, previous)) {
			fprintf(stderr, "[ERROR] [Aqua] [Input] Failed to restore mouse mode: %s\n", SDL_GetError());
		}
		input_manager->properties.mouse_captured = SDL_GetWindowRelativeMouseMode(window);
		return false;
	}
	input_manager->properties = properties;
	return true;
}

void aqua_input_manager_imgui_update(AquaInputManager* input_manager) {
	if (input_manager->properties.show_properties_window) { aqua_input_manager_imgui_properties_window(input_manager); }
}

void aqua_input_manager_imgui_properties_window(AquaInputManager* input_manager) {
	AquaInputManagerProperties properties = input_manager->properties;
	igBegin("Input Manager Properties", &properties.show_properties_window, ImGuiWindowFlags_None);
	bool changed = igCheckbox("Mouse Captured", &properties.mouse_captured);
	igEnd();
	if (changed || properties.show_properties_window != input_manager->properties.show_properties_window) {
		(void) aqua_input_manager_set_properties(input_manager, properties);
	}
}

void aqua_input_manager_destroy(AquaInputManager* input_manager) {
	(void) aqua_input_manager_set_properties(input_manager, AQUA_INPUT_DEFAULT_PROPERTIES);
}
