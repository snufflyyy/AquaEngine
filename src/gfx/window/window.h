#pragma once
#include <aqua/window.h>
#include <aqua/event.h>
#include <SDL3/SDL.h>

struct AquaWindow {
	SDL_Window* sdl_window;
	AquaWindowProperties properties;
};

void aqua_window_event(AquaWindow* window, const AquaEvent* event);
void aqua_window_swap(AquaWindow* window);
void aqua_window_imgui_update(AquaWindow* window);
void aqua_window_imgui_properties_window(AquaWindow* window);
