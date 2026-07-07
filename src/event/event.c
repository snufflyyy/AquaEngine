#include "event.h"
#include "SDL3/SDL_events.h"

#include <SDL3/SDL.h>

bool aqua_event_poll(AquaEvent* event) {
	if (!SDL_PollEvent(&event->sdl_event)) {
		return false;
	}

	switch (event->sdl_event.type) {
		case SDL_EVENT_QUIT: event->type = AQUA_EVENT_TYPE_EXIT; break;
		case SDL_EVENT_WINDOW_RESIZED: {
			event->type = AQUA_EVENT_TYPE_WINDOW_RESIZED;
			event->window = (AquaEventWindow) {
				.new_width = (u32) event->sdl_event.window.data1,
				.new_height = (u32) event->sdl_event.window.data2,
			};
		} break;
		default: event->type = AQUA_EVENT_TYPE_NONE; break;
	}

	return true;
}
