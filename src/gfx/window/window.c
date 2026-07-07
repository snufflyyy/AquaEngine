#include "gfx/window/window.h"
#include "SDL3/SDL_video.h"
#include "event/event.h"

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include <glad/glad.h>
#include <SDL3/SDL.h>
#include <cglm/cglm.h>

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include <cimgui.h>
#include <cimgui_impl.h>

AquaWindow* aqua_window_create(AquaWindowProperties properties) {
	AquaWindow* window = (AquaWindow*) malloc(sizeof(AquaWindow));
	if (!window) {
		fprintf(stderr, "[ERROR] [Aqua] [Window] Failed to allocate memory for window!\n");
		return NULL;
	}

    window->properties = properties;

    SDL_WindowFlags sdl_window_flags = SDL_WINDOW_OPENGL;
    if (window->properties.resizeable) { sdl_window_flags |= SDL_WINDOW_RESIZABLE; }
    if (window->properties.fullscreen) { sdl_window_flags |= SDL_WINDOW_FULLSCREEN; }

   	window->sdl_window = SDL_CreateWindow(properties.title, (i32) properties.width, (i32) properties.height, sdl_window_flags);
    if (!window->sdl_window) {
   		fprintf(stderr, "[ERROR] [Aqua] [Window] Failed to create SDL window!\n");
       	free(window);
       	return NULL;
    }

    #ifndef NDEBUG
	    printf("[INFO] [Aqua] [Window] Created Window\n");
	#endif

	return window;
}

void aqua_window_event(AquaWindow* window, AquaEvent* event) {
    switch (event->type) {
		case AQUA_EVENT_TYPE_WINDOW_RESIZED: {
			window->properties.width = (u32) event->window.new_width;
			window->properties.height = (u32) event->window.new_height;
		} break;
		default: break;
    }
}

void aqua_window_swap(AquaWindow* window) {
	SDL_GL_SwapWindow(window->sdl_window);
}

void aqua_window_imgui_update(AquaWindow* window) {
	if (window->properties.show_properties_window) {
		aqua_window_imgui_properties_window(window);
	}
}

void aqua_window_imgui_properties_window(AquaWindow* window) {
	igBegin("Window Properties", &window->properties.show_properties_window, ImGuiWindowFlags_None);
	igText("Size: %ux%u", window->properties.width, window->properties.height);
	igSeparator();
	if (igCheckbox("Resizeable", &window->properties.resizeable)) { aqua_window_set_resizeable(window, window->properties.resizeable); }
	if (igCheckbox("Fullscreen", &window->properties.fullscreen)) { aqua_window_set_fullscreen(window, window->properties.fullscreen); }
	igEnd();
}

void aqua_window_set_resizeable(AquaWindow* window, bool value) {
	SDL_SetWindowResizable(window->sdl_window, value);
	window->properties.resizeable = value;
}

void aqua_window_set_fullscreen(AquaWindow* window, bool value) {
	SDL_SetWindowFullscreen(window->sdl_window, value);
	window->properties.fullscreen = value;
}

void aqua_window_set_title(AquaWindow* window, const char* title) {
	SDL_SetWindowTitle(window->sdl_window, title);
	window->properties.title = (char*) title;
}

void aqua_window_get_size(AquaWindow* window, u32* width, u32* height) {
	SDL_GetWindowSize(window->sdl_window, (int*) width, (int*) height);
}

void aqua_window_destroy(AquaWindow* window) {
	if (!window) { return; }

   	SDL_DestroyWindow(window->sdl_window);
    free(window);

    #ifndef NDEBUG
    	printf("[INFO] [Aqua] [Window] Window Destroyed\n");
    #endif
}
