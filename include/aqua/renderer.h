#pragma once
#include "types.h"
#include "camera.h"
#include "model-instance.h"
#include "window.h"

typedef struct AquaRendererProperties {
	bool vsync;
	bool wireframe;
	bool rainbow_clear_color;
	f32 rainbow_clear_color_speed;
	AquaColor clear_color;
	bool show_properties_window;
} AquaRendererProperties;
static const AquaRendererProperties AQUA_RENDERER_DEFAULT_PROPERTIES = {
	.vsync = true, .rainbow_clear_color_speed = 2.0f,
	.clear_color = {0.0f, 0.0f, 0.0f, 1.0f},
};

typedef struct AquaRendererInfo {
	// Borrowed until renderer destruction.
	const char* gl_video_driver;
	const char* gl_vendor;
	const char* gl_renderer;
	const char* gl_opengl_version;
	const char* gl_glsl_version;
} AquaRendererInfo;

typedef struct AquaMaterialProperties {
	AquaShaderHandle shader; // INVALID selects the built-in material shader.
	AquaTextureHandle texture; // INVALID selects the default white texture.
	AquaColor color;
	f32 color_strength;
} AquaMaterialProperties;
static const AquaMaterialProperties AQUA_MATERIAL_DEFAULT_PROPERTIES = {
	.shader = AQUA_SHADER_HANDLE_INVALID, .texture = AQUA_TEXTURE_HANDLE_INVALID,
	.color = {1.0f, 1.0f, 1.0f, 1.0f},
};

typedef struct AquaModelInfo {
	bool valid;
	u32 submeshes_count, materials_count, textures_count;
} AquaModelInfo;
typedef struct AquaTextureInfo {
	bool valid;
	u32 width, height;
} AquaTextureInfo;
typedef struct AquaMeshInfo {
	bool valid;
	u32 vertices_count, indices_count;
} AquaMeshInfo;
typedef struct AquaRendererStats {
	u32 models_count, materials_count, meshes_count, textures_count, shaders_count;
} AquaRendererStats;

// Do not destroy a renderer borrowed from an engine context.
AquaRenderer* aqua_renderer_create(AquaRendererProperties properties, AquaWindow* window);
void aqua_renderer_destroy(AquaRenderer* renderer);
AquaRendererProperties aqua_renderer_get_properties(const AquaRenderer* renderer);
// Validate first. Backend failures return false; query again for accepted settings.
bool aqua_renderer_set_properties(AquaRenderer* renderer, AquaRendererProperties properties);
AquaRendererInfo aqua_renderer_get_info(const AquaRenderer* renderer);
AquaRendererStats aqua_renderer_get_stats(const AquaRenderer* renderer);
bool aqua_renderer_set_vsync(AquaRenderer* renderer, bool value);
bool aqua_renderer_set_wireframe(AquaRenderer* renderer, bool value);
bool aqua_renderer_set_clear_color(AquaRenderer* renderer, AquaColor color);
void aqua_renderer_clear(void);

AquaShaderHandle aqua_renderer_create_shader(AquaRenderer* renderer, const char* vertex_path, const char* fragment_path);
bool aqua_renderer_reload_shader(AquaRenderer* renderer, AquaShaderHandle shader);
void aqua_renderer_destroy_shader(AquaRenderer* renderer, AquaShaderHandle shader);
AquaTextureHandle aqua_renderer_create_texture(AquaRenderer* renderer, const char* image_path);
AquaTextureHandle aqua_renderer_create_texture_from_memory(AquaRenderer* renderer, const u8* buffer, u32 length);
AquaTextureHandle aqua_renderer_create_texture_from_pixels(AquaRenderer* renderer, const u8* pixels, u32 width, u32 height);
AquaTextureInfo aqua_renderer_get_texture_info(const AquaRenderer* renderer, AquaTextureHandle texture);
void aqua_renderer_destroy_texture(AquaRenderer* renderer, AquaTextureHandle texture);
AquaMeshHandle aqua_renderer_create_mesh(AquaRenderer* renderer, const AquaVertex* vertices, u32 vertices_count, const u32* indices, u32 indices_count);
AquaMeshInfo aqua_renderer_get_mesh_info(const AquaRenderer* renderer, AquaMeshHandle mesh);
void aqua_renderer_destroy_mesh(AquaRenderer* renderer, AquaMeshHandle mesh);

AquaMaterialHandle aqua_renderer_create_material(AquaRenderer* renderer, AquaTextureHandle texture, AquaColor color);
// Invalid handles return default properties; check validity before editing.
bool aqua_renderer_is_material_valid(const AquaRenderer* renderer, AquaMaterialHandle material);
AquaMaterialProperties aqua_renderer_get_material_properties(const AquaRenderer* renderer, AquaMaterialHandle material);
bool aqua_renderer_set_material_properties(AquaRenderer* renderer, AquaMaterialHandle material, AquaMaterialProperties properties);
void aqua_renderer_set_material_shader(AquaRenderer* renderer, AquaMaterialHandle material, AquaShaderHandle shader);
void aqua_renderer_destroy_material(AquaRenderer* renderer, AquaMaterialHandle material);

// Models own their imported resources. Repeated loads are not cached.
AquaModelHandle aqua_renderer_create_model(AquaRenderer* renderer, const char* path);
AquaModelInfo aqua_renderer_get_model_info(const AquaRenderer* renderer, AquaModelHandle model);
// Detach all instances before unloading. Stale indices can alias replacement models.
void aqua_renderer_destroy_model(AquaRenderer* renderer, AquaModelHandle model);

// The camera is borrowed until unbound; bind NULL before destroying it.
void aqua_renderer_bind_camera(AquaRenderer* renderer, AquaCamera* camera);
void aqua_renderer_bind_material(AquaRenderer* renderer, AquaMaterialHandle material);
void aqua_renderer_draw_mesh(AquaRenderer* renderer, AquaMeshHandle mesh, mat4 transform);
void aqua_renderer_draw_submesh(AquaRenderer* renderer, AquaMeshHandle mesh, u32 start, u32 count, mat4 transform);
void aqua_renderer_draw_static_model(AquaRenderer* renderer, AquaModelHandle model, mat4 transform);
void aqua_renderer_draw_model_instance(AquaRenderer* renderer, const AquaModelInstance* instance);
