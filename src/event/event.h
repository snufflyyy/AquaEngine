#pragma once

#include <stdbool.h>

#include <SDL3/SDL.h>

#include "utils/base-types.h"

typedef enum AquaEventType {
	AQUA_EVENT_TYPE_NONE,
	AQUA_EVENT_TYPE_EXIT,
	AQUA_EVENT_TYPE_WINDOW_RESIZED,
} AquaEventType;

typedef struct AquaEventWindow {
	u32 new_width;
	u32 new_height;
} AquaEventWindow;

typedef struct AquaEvent {
	SDL_Event sdl_event;

	AquaEventType type;

	AquaEventWindow window;
} AquaEvent;

struct AquaEngineContext;

bool aqua_event_poll(AquaEvent* event);
