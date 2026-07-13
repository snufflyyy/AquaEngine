#pragma once

#include "gfx/shader/shader.h"
#include "resource-manager.h"
#include <SDL3/SDL.h>
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include <cimgui.h>
#include <cimgui_impl.h>

#include "utils/base-types.h"
#include "gfx/window/window.h"
#include "gfx/color.h"
#include "gfx/material/material.h"
#include "gfx/camera/camera.h"

typedef struct AquaRendererCreateProperties {
	bool vsync;

	bool rainbow_clear_color; // for fun
	float rainbow_clear_color_speed;
	AquaColor clear_color;

	bool show_performance_window;

	bool show_properties_window;
} AquaRendererCreateProperties;

const static AquaRendererCreateProperties AQUA_RENDERER_DEFAULT_CREATE_PROPERTIES = {
	.vsync = true,

	.rainbow_clear_color = false,
	.rainbow_clear_color_speed = 2.0f,
	.clear_color = { 0.0f, 0.0f, 0.0f },

	.show_performance_window = false,

	.show_properties_window = false,
};

typedef struct AquaRendererProperties {
	bool vsync;

	bool rainbow_clear_color; // for fun
	float rainbow_clear_color_speed;
	AquaColor clear_color;

	char* gl_video_driver;
	char* gl_vender;
	char* gl_renderer;
	char* gl_opengl_version;
	char* gl_glsl_version;

	bool show_performance_window;

	bool imgui_process_input;

	bool show_properties_window;
} AquaRendererProperties;

typedef struct AquaRenderer {
	SDL_GLContext gl_context;
	AquaShaderHandle current_shader;

	AquaRendererResourceManager resource_manager;

	u64 last_performance_counter;
	double performance_frequency;

	float fps;
	double delta_time;

	ImGuiContext* imgui_context;
	bool started_imgui_frame;

	AquaRendererProperties properties;
} AquaRenderer;

AquaRenderer* aqua_renderer_create(AquaRendererCreateProperties create_properties, AquaWindow* window);

void aqua_renderer_event(AquaRenderer* renderer, AquaEvent* event);
void aqua_renderer_update(AquaRenderer* renderer);

void aqua_renderer_set_vsync(AquaRenderer* renderer, bool value);
void aqua_renderer_set_clear_color(AquaColor color);
void aqua_renderer_clear(void);

AquaShaderHandle aqua_renderer_create_shader(AquaRenderer* renderer, const char* vertex_shader_source_path, const char* fragment_shader_source_path);
AquaMeshHandle aqua_renderer_create_mesh(AquaRenderer* renderer, AquaVertex* vertices, u32 vertices_count, GLuint* indices, u32 indices_count);

void aqua_renderer_draw_mesh(AquaRenderer* renderer, AquaCamera* camera, AquaMeshHandle mesh, AquaMaterial* material);

void aqua_renderer_imgui_begin(AquaRenderer* renderer);
void aqua_renderer_imgui_update(AquaRenderer* renderer);
void aqua_renderer_imgui_properties_window(AquaRenderer* renderer);
void aqua_renderer_imgui_performance_window(AquaRenderer* renderer);
void aqua_renderer_imgui_end(AquaRenderer* renderer);
void aqua_renderer_imgui_draw(AquaRenderer* renderer);

void aqua_renderer_destroy(AquaRenderer* renderer);
