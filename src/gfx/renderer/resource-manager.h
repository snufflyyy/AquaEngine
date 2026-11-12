#pragma once

#include "gfx/shader/shader.h"
#include "gfx/texture/texture.h"
#include "gfx/mesh/mesh.h"
#include <aqua/types.h>
#include <aqua/debug.h>

#define AQUA_RENDERER_RESOURCE_MANAGER_MAX_SHADER_COUNT 1024
#define AQUA_RENDERER_RESOURCE_MANAGER_MAX_TEXTURE_COUNT 1024
#define AQUA_RENDERER_RESOURCE_MANAGER_MAX_MESH_COUNT 1024

// Fixed slots per resource type. Destroyed slots are reused by the next create of that type,
// so a handle kept after destroying its resource may later refer to a different one.
// Check handles with the is_*_valid functions: an index below *_count is not necessarily live.
typedef struct AquaRendererResourceManager {
	AquaShader shaders[AQUA_RENDERER_RESOURCE_MANAGER_MAX_SHADER_COUNT];
	bool shaders_in_use[AQUA_RENDERER_RESOURCE_MANAGER_MAX_SHADER_COUNT];
	u32 shaders_count; // live shaders

	AquaTexture textures[AQUA_RENDERER_RESOURCE_MANAGER_MAX_TEXTURE_COUNT];
	bool textures_in_use[AQUA_RENDERER_RESOURCE_MANAGER_MAX_TEXTURE_COUNT];
	u32 textures_count; // live textures

	AquaMesh meshes[AQUA_RENDERER_RESOURCE_MANAGER_MAX_MESH_COUNT];
	bool meshes_in_use[AQUA_RENDERER_RESOURCE_MANAGER_MAX_MESH_COUNT];
	u32 meshes_count; // live meshes

	AquaRendererResourceManagerProperties properties;
} AquaRendererResourceManager;

void aqua_renderer_resource_manager_init(AquaRendererResourceManager* resource_manager);

AquaShaderHandle aqua_renderer_resource_manager_create_shader(AquaRendererResourceManager* resource_manager, const char* vertex_shader_source_path, const char* fragment_shader_source_path);
AquaTextureHandle aqua_renderer_resource_manager_create_texture(AquaRendererResourceManager* resource_manager, const char* image_path);
AquaTextureHandle aqua_renderer_resource_manager_create_texture_from_memory(AquaRendererResourceManager* resource_manager, const u8* buffer, u32 buffer_length);
AquaTextureHandle aqua_renderer_resource_manager_create_texture_from_pixels(AquaRendererResourceManager* resource_manager, const u8* rgba_pixels, u32 width, u32 height);
AquaMeshHandle aqua_renderer_resource_manager_create_mesh(AquaRendererResourceManager* resource_manager, const AquaVertex* vertices, u32 vertices_count, const GLuint* indices, u32 indices_count);

// release the GPU resource and free its slot; invalid handles are a no-op, destroyed ones are logged
void aqua_renderer_resource_manager_destroy_shader(AquaRendererResourceManager* resource_manager, AquaShaderHandle shader);
void aqua_renderer_resource_manager_destroy_texture(AquaRendererResourceManager* resource_manager, AquaTextureHandle texture);
void aqua_renderer_resource_manager_destroy_mesh(AquaRendererResourceManager* resource_manager, AquaMeshHandle mesh);

bool aqua_renderer_resource_manager_is_shader_valid(const AquaRendererResourceManager* resource_manager, AquaShaderHandle shader);
bool aqua_renderer_resource_manager_is_texture_valid(const AquaRendererResourceManager* resource_manager, AquaTextureHandle texture);
bool aqua_renderer_resource_manager_is_mesh_valid(const AquaRendererResourceManager* resource_manager, AquaMeshHandle mesh);

void aqua_renderer_resource_manager_imgui_update(AquaRendererResourceManager* resource_manager);
void aqua_renderer_resource_manager_imgui_properties_window(AquaRendererResourceManager* resource_manager);

// destroys every live resource
void aqua_renderer_resource_manager_destroy(AquaRendererResourceManager* resource_manager);
