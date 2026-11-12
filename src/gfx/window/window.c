#include "window.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include <cimgui.h>

static bool window_properties_valid(const AquaWindowProperties properties) {
	return properties.title && properties.width > 0 && properties.height > 0
		&& properties.width <= INT_MAX && properties.height <= INT_MAX;
}

static bool window_error(const char* operation) {
	fprintf(stderr, "[ERROR] [Aqua] [Window] %s failed: %s\n", operation, SDL_GetError());
	return false;
}

AquaWindow* aqua_window_create(AquaWindowProperties properties) {
	if (!window_properties_valid(properties)) {
		fprintf(stderr, "[ERROR] [Aqua] [Window] Invalid window properties!\n");
		return NULL;
	}

	AquaWindow* window = calloc(1, sizeof(*window));
	if (!window) { return NULL; }

	window->sdl_window = SDL_CreateWindow(properties.title, (int) properties.width, (int) properties.height, SDL_WINDOW_OPENGL);
	if (!window->sdl_window) {
		(void) window_error("Creating window");
		free(window);
		return NULL;
	}

	if (!aqua_window_set_properties(window, properties)) {
		aqua_window_destroy(window);
		return NULL;
	}

	#ifndef NDEBUG
		printf("[INFO] [Aqua] [Window] Created Window\n");
	#endif

	return window;
}

AquaWindowProperties aqua_window_get_properties(const AquaWindow* window) {
	AquaWindowProperties properties = window->properties;
	properties.title = aqua_window_get_title(window);
	return properties;
}

bool aqua_window_set_properties(AquaWindow* window, AquaWindowProperties properties) {
	if (!window || !window_properties_valid(properties)) {
		fprintf(stderr, "[ERROR] [Aqua] [Window] Invalid window properties!\n");
		return false;
	}

	if (!aqua_window_set_title(window, properties.title)) { return false; }
	if (properties.fullscreen != window->properties.fullscreen) {
		if (!SDL_SetWindowFullscreen(window->sdl_window, properties.fullscreen)) { return window_error("Setting fullscreen"); }
		window->properties.fullscreen = properties.fullscreen;
	}

	if (properties.width != window->properties.width || properties.height != window->properties.height) {
		if (!SDL_SetWindowSize(window->sdl_window, (int) properties.width, (int) properties.height)) { return window_error("Setting size"); }
		window->properties.width = properties.width;
		window->properties.height = properties.height;
	}

	if (properties.resizable != window->properties.resizable) {
		if (!SDL_SetWindowResizable(window->sdl_window, properties.resizable)) { return window_error("Setting resizable"); }
		window->properties.resizable = properties.resizable;
	}

	window->properties.show_properties_window = properties.show_properties_window;

	return true;
}

void aqua_window_get_pixel_size(const AquaWindow* window, u32* width, u32* height) {
	int w = 0, h = 0;
	if (!SDL_GetWindowSizeInPixels(window->sdl_window, &w, &h)) { (void) window_error("Querying pixel size"); }
	if (width) { *width = w > 0 ? (u32) w : 0; }
	if (height) { *height = h > 0 ? (u32) h : 0; }
}

void aqua_window_get_size(const AquaWindow* window, u32* width, u32* height) {
	int w = 0, h = 0;
	if (!SDL_GetWindowSize(window->sdl_window, &w, &h)) { (void) window_error("Querying size"); }
	if (width) { *width = w > 0 ? (u32) w : 0; }
	if (height) { *height = h > 0 ? (u32) h : 0; }
}

const char* aqua_window_get_title(const AquaWindow* window) {
	return SDL_GetWindowTitle(window->sdl_window);
}

bool aqua_window_set_title(AquaWindow* window, const char* title) {
	if (!window || !title) {
		fprintf(stderr, "[ERROR] [Aqua] [Window] Invalid window title!\n");
		return false;
	}
	if (!SDL_SetWindowTitle(window->sdl_window, title)) { return window_error("Setting title"); }
	return true;
}

bool aqua_window_set_resizable(AquaWindow* window, bool value) {
	AquaWindowProperties properties = aqua_window_get_properties(window);
	properties.resizable = value;
	return aqua_window_set_properties(window, properties);
}

bool aqua_window_set_fullscreen(AquaWindow* window, bool value) {
	AquaWindowProperties properties = aqua_window_get_properties(window);
	properties.fullscreen = value;
	return aqua_window_set_properties(window, properties);
}

void aqua_window_event(AquaWindow* window, const AquaEvent* event) {
	if (event->type == AQUA_EVENT_TYPE_WINDOW_RESIZED && event->window.new_width > 0 && event->window.new_height > 0) {
		window->properties.width = event->window.new_width;
		window->properties.height = event->window.new_height;
	} else if (event->type == AQUA_EVENT_TYPE_WINDOW_LEAVE_FULLSCREEN) {
		window->properties.fullscreen = false;
	}
}

void aqua_window_swap(AquaWindow* window) {
	SDL_GL_SwapWindow(window->sdl_window);
}

void aqua_window_imgui_update(AquaWindow* window) {
	if (window->properties.show_properties_window) { aqua_window_imgui_properties_window(window); }
}

void aqua_window_imgui_properties_window(AquaWindow* window) {
	AquaWindowProperties properties = aqua_window_get_properties(window);
	igBegin("Window Properties", &properties.show_properties_window, ImGuiWindowFlags_None);
	u32 width = 0, height = 0;
	aqua_window_get_pixel_size(window, &width, &height);
	igText("Logical size: %ux%u", properties.width, properties.height);
	igText("Pixel size: %ux%u", width, height);
	bool changed = igCheckbox("Resizable", &properties.resizable);
	changed |= igCheckbox("Fullscreen", &properties.fullscreen);
	igEnd();
	if (changed || properties.show_properties_window != window->properties.show_properties_window) {
		(void) aqua_window_set_properties(window, properties);
	}
}

void aqua_window_destroy(AquaWindow* window) {
	if (!window) { return; }
	SDL_DestroyWindow(window->sdl_window);
	free(window);

	#ifndef NDEBUG
		printf("[INFO] [Aqua] [Window] Window Destroyed\n");
	#endif
}
