#pragma once

#include <glad/glad.h>
#include <SDL3/SDL.h>
#include <cglm/cglm.h>

#include "utils/base-types.h"
#include "event/event.h"

typedef struct AquaWindowProperties {
	u32 width, height;
	char* title;
	bool resizeable;
	bool fullscreen;
	bool mouse_captured;

	bool show_properties_window;
} AquaWindowProperties;

const static AquaWindowProperties AQUA_WINDOW_DEFAULT_WINDOW_PROPERTIES = {
	.width = 1280,
	.height = 720,
	.title = "Aqua Engine",
	.resizeable = false,
	.fullscreen = false,
	.mouse_captured = false,
	.show_properties_window = false,
};

typedef struct AquaWindow {
	SDL_Window* sdl_window;

	AquaWindowProperties properties;
} AquaWindow;

AquaWindow* aqua_window_create(AquaWindowProperties properties);

void aqua_window_event(AquaWindow* window, AquaEvent* event);

void aqua_window_swap(AquaWindow* window);

void aqua_window_get_size(AquaWindow* window, u32* width, u32* height);

void aqua_window_set_title(AquaWindow* window, const char* title);
void aqua_window_set_resizeable(AquaWindow* window, bool value);
void aqua_window_set_fullscreen(AquaWindow* window, bool value);

void aqua_window_imgui_update(AquaWindow* window);
void aqua_window_imgui_properties_window(AquaWindow* window);

void aqua_window_destroy(AquaWindow* window);
