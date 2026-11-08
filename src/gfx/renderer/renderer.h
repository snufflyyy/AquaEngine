#pragma once

#include "gfx/shader/shader.h"
#include "gfx/model/model.h"
#include "resource-manager.h"
#include <SDL3/SDL.h>
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include <cimgui.h>
#include <cimgui_impl.h>

#include "utils/base-types.h"
#include "gfx/window/window.h"
#include "gfx/color.h"
#include "gfx/material/material.h"
#include "gfx/material/material-manager.h"
#include "gfx/camera/camera.h"

typedef struct AquaRendererCreateProperties {
	bool vsync;

	bool rainbow_clear_color; // for fun
	float rainbow_clear_color_speed;
	AquaColor clear_color;

	bool show_performance_window;

	bool show_properties_window;
} AquaRendererCreateProperties;

static const AquaRendererCreateProperties AQUA_RENDERER_DEFAULT_CREATE_PROPERTIES = {
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

	AquaRendererResourceManager resource_manager;
	AquaMaterialManager material_manager;

	AquaCamera* current_camera;
	AquaShaderHandle current_shader; // set by aqua_renderer_bind_material

	AquaShaderHandle base_material_shader;
	AquaTextureHandle default_texture; // 1x1 white, bound when a material has no texture
	AquaMaterialHandle default_material; // base shader + white texture, bound for invalid material handles

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
AquaTextureHandle aqua_renderer_create_texture(AquaRenderer* renderer, const char* image_path);
AquaTextureHandle aqua_renderer_create_texture_from_memory(AquaRenderer* renderer, const u8* buffer, u32 buffer_length);
AquaTextureHandle aqua_renderer_create_texture_from_pixels(AquaRenderer* renderer, const u8* rgba_pixels, u32 width, u32 height);
AquaMeshHandle aqua_renderer_create_mesh(AquaRenderer* renderer, AquaVertex* vertices, u32 vertices_count, GLuint* indices, u32 indices_count);

// free the resource and its slot; the base material shader and default texture are refused.
// materials still pointing at a destroyed texture draw white; at a destroyed shader they are skipped
void aqua_renderer_destroy_shader(AquaRenderer* renderer, AquaShaderHandle shader);
void aqua_renderer_destroy_texture(AquaRenderer* renderer, AquaTextureHandle texture);
void aqua_renderer_destroy_mesh(AquaRenderer* renderer, AquaMeshHandle mesh);

// materials use the base material shader; change it with aqua_renderer_set_material_shader
AquaMaterialHandle aqua_renderer_create_material(AquaRenderer* renderer, AquaTextureHandle texture, AquaColor color);
void aqua_renderer_destroy_material(AquaRenderer* renderer, AquaMaterialHandle material);
// NULL for invalid or destroyed handles; the pointer is valid until the material is destroyed
AquaMaterial* aqua_renderer_get_material(AquaRenderer* renderer, AquaMaterialHandle material);
void aqua_renderer_set_material_shader(AquaRenderer* renderer, AquaMaterialHandle material, AquaShaderHandle shader);

void aqua_renderer_bind_camera(AquaRenderer* renderer, AquaCamera* camera);
// invalid or destroyed handles bind the default material
void aqua_renderer_bind_material(AquaRenderer* renderer, AquaMaterialHandle material);

// transform is the model (object-to-world) matrix
void aqua_renderer_draw_mesh(AquaRenderer* renderer, AquaMeshHandle mesh, mat4 transform);
void aqua_renderer_draw_submesh(AquaRenderer* renderer, AquaMeshHandle mesh, u32 indices_start, u32 indices_count, mat4 transform);

// draws with model->transform

void aqua_renderer_draw_static_model(AquaRenderer* renderer, AquaStaticModel* model);

void aqua_renderer_imgui_begin(AquaRenderer* renderer);
void aqua_renderer_imgui_update(AquaRenderer* renderer);
void aqua_renderer_imgui_properties_window(AquaRenderer* renderer);
void aqua_renderer_imgui_performance_window(AquaRenderer* renderer);
void aqua_renderer_imgui_end(AquaRenderer* renderer);
void aqua_renderer_imgui_draw(AquaRenderer* renderer);

void aqua_renderer_destroy(AquaRenderer* renderer);
