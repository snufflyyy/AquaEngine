#pragma once
#include "types.h"

typedef struct AquaInputManagerProperties {
	bool mouse_captured;
	bool show_properties_window;
} AquaInputManagerProperties;
static const AquaInputManagerProperties AQUA_INPUT_DEFAULT_PROPERTIES = {0};

AquaInputManagerProperties aqua_input_get_properties(const AquaEngineContext* context);
bool aqua_input_set_properties(AquaEngineContext* context, AquaInputManagerProperties properties);
bool aqua_input_is_mouse_captured(const AquaEngineContext* context);
bool aqua_input_set_mouse_captured(AquaEngineContext* context, bool captured);
