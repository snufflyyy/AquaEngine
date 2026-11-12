#pragma once
#include "types.h"

typedef struct AquaWindowProperties {
	u32 width, height; // Requested logical size, not framebuffer pixels.
	const char* title;
	bool resizable;
	bool fullscreen;
	bool show_properties_window;
} AquaWindowProperties;

static const AquaWindowProperties AQUA_WINDOW_DEFAULT_PROPERTIES = {
	.width = 1280, .height = 720, .title = "Aqua Engine",
};

// Standalone window creation requires initialized platform services.
// Do not destroy a window borrowed from an engine context.
AquaWindow* aqua_window_create(AquaWindowProperties properties);
void aqua_window_destroy(AquaWindow* window);
// title is borrowed until the title changes or the window is destroyed.
AquaWindowProperties aqua_window_get_properties(const AquaWindow* window);
// SDL copies incoming title text. Backend failures can partially apply; query again on false.
bool aqua_window_set_properties(AquaWindow* window, AquaWindowProperties properties);
void aqua_window_get_pixel_size(const AquaWindow* window, u32* width, u32* height);
void aqua_window_get_size(const AquaWindow* window, u32* width, u32* height);
// Borrowed SDL-owned text, valid until the title changes or the window is destroyed.
const char* aqua_window_get_title(const AquaWindow* window);
// SDL copies the text. title must not be NULL.
bool aqua_window_set_title(AquaWindow* window, const char* title);
bool aqua_window_set_resizable(AquaWindow* window, bool value);
bool aqua_window_set_fullscreen(AquaWindow* window, bool value);
