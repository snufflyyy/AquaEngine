#pragma once

#include "aqua/types.h"
#include "aqua/event.h"
#include "aqua/project.h"
#include "aqua/window.h"
#include "aqua/clock.h"
#include "aqua/input.h"
#include "aqua/camera.h"
#include "aqua/fly-camera.h"
#include "aqua/renderer.h"
#include "aqua/model-instance.h"

typedef struct AquaContextProperties {
	bool show_properties_window;
} AquaContextProperties;
static const AquaContextProperties AQUA_CONTEXT_DEFAULT_PROPERTIES = { .show_properties_window = true };

typedef struct AquaEngineContextInitializeProperties {
	AquaClockProperties clock_properties;
	AquaWindowProperties window_properties;
	AquaRendererProperties renderer_properties;
	AquaInputManagerProperties input_properties;
	AquaContextProperties context_properties;
} AquaEngineContextInitializeProperties;
static const AquaEngineContextInitializeProperties AQUA_ENGINE_CONTEXT_DEFAULT_INITIALIZE_PROPERTIES = {
	.clock_properties = AQUA_CLOCK_DEFAULT_PROPERTIES,
	.window_properties = AQUA_WINDOW_DEFAULT_PROPERTIES,
	.renderer_properties = AQUA_RENDERER_DEFAULT_PROPERTIES,
	.input_properties = AQUA_INPUT_DEFAULT_PROPERTIES,
	.context_properties = AQUA_CONTEXT_DEFAULT_PROPERTIES,
};

// project and its user data are borrowed until destruction. A failed initialize
// must clean up application-owned allocations; its destroy callback is not called.
AquaEngineContext* aqua_engine_context_create(AquaProject* project, AquaEngineContextInitializeProperties properties);
void aqua_engine_context_destroy(AquaEngineContext* context);
void aqua_engine_run(AquaEngineContext* context);
// Services are borrowed and must not be destroyed separately.
AquaRenderer* aqua_engine_context_get_renderer(AquaEngineContext* context);
AquaWindow* aqua_engine_context_get_window(AquaEngineContext* context);
void* aqua_engine_context_get_project_data(AquaEngineContext* context);
AquaContextProperties aqua_engine_context_get_properties(const AquaEngineContext* context);
bool aqua_engine_context_set_properties(AquaEngineContext* context, AquaContextProperties properties);
f64 aqua_engine_context_get_delta_time(const AquaEngineContext* context);
void aqua_engine_context_request_exit(AquaEngineContext* context);
