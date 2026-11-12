#pragma once
#include <aqua/input.h>
#include "gfx/window/window.h"

typedef struct AquaInputManager {
	AquaWindow* window_ptr;
	AquaInputManagerProperties properties;
} AquaInputManager;

AquaInputManager aqua_input_manager_create(AquaWindow* window);
bool aqua_input_manager_set_properties(AquaInputManager* input_manager, AquaInputManagerProperties properties);
void aqua_input_manager_imgui_update(AquaInputManager* input_manager);
void aqua_input_manager_imgui_properties_window(AquaInputManager* input_manager);
void aqua_input_manager_destroy(AquaInputManager* input_manager);
