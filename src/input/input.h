#pragma once

#include <stdbool.h>

#include "gfx/window/window.h"

typedef struct AquaInputManagerProperties {
	bool mouse_captured;
	bool show_properties_window;
} AquaInputManagerProperties;

typedef struct AquaInputManager {
	AquaWindow* window_ptr;

	AquaInputManagerProperties properties;
} AquaInputManager;

AquaInputManager aqua_input_manager_create(AquaWindow* window);

void aqua_input_manager_set_mouse_captured(AquaInputManager* input_manager, bool value);

void aqua_input_manager_imgui_update(AquaInputManager* input_manager);
void aqua_input_manager_imgui_properties_window(AquaInputManager* input_manager);

void aqua_input_manager_destroy(AquaInputManager* input_manager);
