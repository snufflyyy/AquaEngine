#pragma once
#include <aqua.h>
#include "clock/clock.h"
#include "input/input.h"

struct AquaEngineContext {
	AquaProject* project;
	bool running;
	AquaClock clock;
	AquaWindow* window;
	AquaRenderer* renderer;
	AquaInputManager input_manager;
	AquaContextProperties properties;
};

void aqua_engine_context_event(AquaEngineContext* context, const AquaEvent* event);
void aqua_engine_context_update(AquaEngineContext* context);
void aqua_engine_imgui_update(AquaEngineContext* context);
void aqua_engine_imgui_properties_window(AquaEngineContext* context);
