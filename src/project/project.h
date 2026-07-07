#pragma once

#include "event/event.h"

struct AquaEngineContext;

typedef struct AquaProject {
	char* name;
	char* creator;

	void* project_data_ptr;

	void (*initialize)(struct AquaEngineContext* aqua_context);
	void (*event)(struct AquaEngineContext* aqua_context, AquaEvent* event);
	void (*tick)(struct AquaEngineContext* aqua_context);
	void (*update)(struct AquaEngineContext* aqua_context);
	void (*render)(struct AquaEngineContext* aqua_context);
	void (*destroy)(struct AquaEngineContext* aqua_context);
} AquaProject;
