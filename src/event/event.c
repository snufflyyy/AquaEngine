#include "event.h"
#include "gfx/renderer/renderer.h"

static AquaEvent aqua_event_translate(const SDL_Event* native_event) {
	AquaEvent event = {0};

	switch (native_event->type) {
		case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
		case SDL_EVENT_QUIT: event.type = AQUA_EVENT_TYPE_EXIT; break;

		case SDL_EVENT_WINDOW_RESIZED:
		case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
			event.type = native_event->type == SDL_EVENT_WINDOW_RESIZED
				? AQUA_EVENT_TYPE_WINDOW_RESIZED : AQUA_EVENT_TYPE_WINDOW_PIXEL_SIZE_CHANGED;
			event.window.new_width = native_event->window.data1 > 0 ? (u32) native_event->window.data1 : 0;
			event.window.new_height = native_event->window.data2 > 0 ? (u32) native_event->window.data2 : 0;
			break;

		case SDL_EVENT_WINDOW_LEAVE_FULLSCREEN: event.type = AQUA_EVENT_TYPE_WINDOW_LEAVE_FULLSCREEN; break;

		case SDL_EVENT_KEY_UP:
		case SDL_EVENT_KEY_DOWN:
			event.type = native_event->type == SDL_EVENT_KEY_UP ? AQUA_EVENT_TYPE_KEY_UP : AQUA_EVENT_TYPE_KEY_DOWN;
			event.key.scancode = (AquaEventKeyScancode) native_event->key.scancode;
			break;

		case SDL_EVENT_MOUSE_MOTION:
			event.type = AQUA_EVENT_TYPE_MOUSE_MOTION;
			event.mouse_motion = (AquaEventMouseMotion) {
				.x = native_event->motion.x, .y = native_event->motion.y,
				.x_relative = native_event->motion.xrel, .y_relative = native_event->motion.yrel,
			};
			break;

		default: break;
	}

	return event;
}

bool aqua_event_poll(AquaEvent* event, AquaRenderer* renderer, bool process_imgui_input) {
	SDL_Event native_event;
	if (!SDL_PollEvent(&native_event)) { return false; }

	renderer->imgui_process_input = process_imgui_input;
	aqua_renderer_imgui_native_event(renderer, &native_event);

	*event = aqua_event_translate(&native_event);

	return true;
}
