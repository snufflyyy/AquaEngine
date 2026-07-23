#pragma once

#include "gfx/shader/shader.h"
#include "gfx/texture/texture.h"
#include "gfx/mesh/mesh.h"
#include "utils/base-types.h"

#define AQUA_RENDERER_RESOURCE_MANAGER_MAX_SHADER_COUNT 1000
#define AQUA_RENDERER_RESOURCE_MANAGER_MAX_TEXTURE_COUNT 1000
#define AQUA_RENDERER_RESOURCE_MANAGER_MAX_MESH_COUNT 1000

typedef struct AquaRendererResourceManagerProperties {
	bool show_properties_window;
} AquaRendererResourceManagerProperties;

typedef struct AquaRendererResourceManager {
	AquaShader shaders[AQUA_RENDERER_RESOURCE_MANAGER_MAX_SHADER_COUNT];
	u32 shaders_count;

	AquaTexture textures[AQUA_RENDERER_RESOURCE_MANAGER_MAX_TEXTURE_COUNT];
	u32 texture_count;

	AquaMesh meshes[AQUA_RENDERER_RESOURCE_MANAGER_MAX_MESH_COUNT];
	u32 meshes_count;

	AquaRendererResourceManagerProperties properties;
} AquaRendererResourceManager;

AquaRendererResourceManager aqua_renderer_resource_manager_create(void);

AquaShaderHandle aqua_renderer_resource_manager_create_shader(AquaRendererResourceManager* resource_manager, const char* vertex_shader_source_path, const char* fragment_shader_source_path);
AquaTextureHandle aqua_renderer_resource_manager_create_texture(AquaRendererResourceManager* resource_manager, const char* image_path);
AquaTextureHandle aqua_renderer_resource_manager_create_texture_from_memory(AquaRendererResourceManager* resource_manager, const u8* buffer, u32 buffer_length);
AquaMeshHandle aqua_renderer_resource_manager_create_mesh(AquaRendererResourceManager* resource_manager, AquaVertex* vertices, u32 vertices_count, GLuint* indices, u32 indices_count);

void aqua_renderer_resource_manager_imgui_update(AquaRendererResourceManager* resource_manager);
void aqua_renderer_resource_manager_imgui_properties_window(AquaRendererResourceManager* resource_manager);

void aqua_renderer_resource_manager_destroy(AquaRendererResourceManager* resource_manager);
