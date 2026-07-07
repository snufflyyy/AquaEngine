#pragma once

#include <stdbool.h>

#include "project/project.h"
#include "tick/tick.h"
#include "gfx/window/window.h"
#include "gfx/renderer/renderer.h"

typedef struct AquaEngineContextInitializeProperties {
	AquaTickHandlerProperties tick_handler_properties;
	AquaWindowProperties window_properties;
	AquaRendererCreateProperties renderer_create_properties;
	bool show_properties_window;
} AquaEngineContextInitializeProperties;

const static AquaEngineContextInitializeProperties AQUA_ENGINE_CONTEXT_DEFAULT_INITIALIZE_PROPERTIES = {
	.tick_handler_properties = AQUA_TICK_HANDLER_DEFAULT_PROPERTIES,
	.window_properties = AQUA_WINDOW_DEFAULT_WINDOW_PROPERTIES,
	.renderer_create_properties = AQUA_RENDERER_DEFAULT_CREATE_PROPERTIES,
	.show_properties_window = true,
};

typedef struct AquaContextProperties {
	bool show_properties_window;
} AquaContextProperties;

typedef struct AquaEngineContext {
	AquaProject project;

	bool running;
	AquaTickHandler tick_handler;

	AquaWindow* window;
	AquaRenderer* renderer;

	AquaContextProperties properties;
} AquaEngineContext;

AquaEngineContext* aqua_engine_context_create(AquaProject* project, AquaEngineContextInitializeProperties properties);
void aqua_engine_context_event(AquaEngineContext* aqua_context, AquaEvent* event);
void aqua_engine_context_tick(AquaEngineContext* aqua_context);
void aqua_engine_context_update(AquaEngineContext* aqua_context);
void aqua_engine_context_destroy(AquaEngineContext* aqua_context);

void aqua_engine_imgui_update(AquaEngineContext* aqua_context);
void aqua_engine_imgui_properties_window(AquaEngineContext* aqua_context);

void aqua_engine_run(AquaEngineContext* aqua_context);
