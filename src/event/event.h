#pragma once

#include <stdbool.h>

#include <SDL3/SDL.h>

#include "utils/base-types.h"

typedef enum AquaEventType {
	AQUA_EVENT_TYPE_NONE,
	AQUA_EVENT_TYPE_EXIT,
	AQUA_EVENT_TYPE_WINDOW_RESIZED,
	AQUA_EVENT_TYPE_MOUSE_MOTION
} AquaEventType;

typedef struct AquaEventWindow {
	u32 new_width;
	u32 new_height;
} AquaEventWindow;

typedef struct AquaEventMouseMotion {
    f32 x, y;
    f32 x_relative, y_relative;
} AquaEventMouseMotion;

typedef struct AquaEvent {
	SDL_Event sdl_event;

	AquaEventType type;

	AquaEventWindow window;
	AquaEventMouseMotion mouse_motion;
} AquaEvent;

bool aqua_event_poll(AquaEvent* event);
