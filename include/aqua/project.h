#pragma once

#include "event.h"

// Descriptor, metadata strings, and user data must outlive the engine context.
// All callbacks are optional. If initialize fails, it must release application-owned
// allocations itself; destroy is only called for a successfully created context.

typedef struct AquaProject {
	const char* name;
	const char* creator;

	bool show_properties_window;

	void* project_data_ptr;

	bool (*initialize)(struct AquaEngineContext* aqua_context);
	void (*event)(struct AquaEngineContext* aqua_context, const AquaEvent* event);
	void (*tick)(struct AquaEngineContext* aqua_context);
	void (*update)(struct AquaEngineContext* aqua_context);
	void (*render)(struct AquaEngineContext* aqua_context);
	void (*imgui_update)(struct AquaEngineContext* aqua_context);
	void (*destroy)(struct AquaEngineContext* aqua_context);
} AquaProject;
