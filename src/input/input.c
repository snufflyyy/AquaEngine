#include "input.h"

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include <cimgui.h>
#include <cimgui_impl.h>

AquaInputManager aqua_input_manager_create(AquaWindow* window) {
	return (AquaInputManager) {
		.window_ptr = window,
		.properties = (AquaInputManagerProperties) {
			.mouse_captured = false,
			.show_properties_window = false,
		}
 	};
}

void aqua_input_manager_set_mouse_captured(AquaInputManager* input_manager, bool value) {
	input_manager->properties.mouse_captured = value;

	SDL_SetWindowRelativeMouseMode(input_manager->window_ptr->sdl_window, value);
 	SDL_SetWindowMouseGrab(input_manager->window_ptr->sdl_window, value);
}

void aqua_input_manager_imgui_update(AquaInputManager* input_manager) {
	if (input_manager->properties.show_properties_window) {
		aqua_input_manager_imgui_properties_window(input_manager);
	}
}

void aqua_input_manager_imgui_properties_window(AquaInputManager* input_manager) {
	igBegin("Input Manager Properties", &input_manager->properties.show_properties_window, ImGuiWindowFlags_None);
	if (igCheckbox("Mouse Captured", &input_manager->properties.mouse_captured)) {
		aqua_input_manager_set_mouse_captured(input_manager, input_manager->properties.mouse_captured);
	}
	igEnd();
}

void aqua_input_manager_destroy(AquaInputManager* input_manager) {

}
