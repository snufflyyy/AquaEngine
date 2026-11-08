#include "resource-manager.h"
#include "gfx/texture/texture.h"
#include "utils/base-types.h"

#include <stdio.h>
#include <string.h>

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include <cimgui.h>
#include <cimgui_impl.h>

#include "gfx/shader/shader.h"
#include "gfx/mesh/mesh.h"

#define RESOURCE_MANAGER_LOG "[Aqua] [Renderer Resource Manager]"

// first unused slot, so slots released by destroyed resources get reused; USIZE_MAX when full
static usize resource_manager_find_free_slot(const bool* in_use, usize capacity) {
	for (usize i = 0; i < capacity; i++) {
		if (!in_use[i]) { return i; }
	}
	return USIZE_MAX;
}

// initialized in place: the struct is several MB, and returning it by value can put a copy on the stack
void aqua_renderer_resource_manager_init(AquaRendererResourceManager* resource_manager) {
	memset(resource_manager, 0, sizeof(*resource_manager));

	#ifndef NDEBUG
		printf("[INFO] [Aqua] [Renderer Resource Manager] Created Renderer Resource Manager\n");
	#endif
}

//
// validity
//

bool aqua_renderer_resource_manager_is_shader_valid(const AquaRendererResourceManager* resource_manager, AquaShaderHandle shader) {
	return shader < AQUA_RENDERER_RESOURCE_MANAGER_MAX_SHADER_COUNT && resource_manager->shaders_in_use[shader];
}

bool aqua_renderer_resource_manager_is_texture_valid(const AquaRendererResourceManager* resource_manager, AquaTextureHandle texture) {
	return texture < AQUA_RENDERER_RESOURCE_MANAGER_MAX_TEXTURE_COUNT && resource_manager->textures_in_use[texture];
}

bool aqua_renderer_resource_manager_is_mesh_valid(const AquaRendererResourceManager* resource_manager, AquaMeshHandle mesh) {
	return mesh < AQUA_RENDERER_RESOURCE_MANAGER_MAX_MESH_COUNT && resource_manager->meshes_in_use[mesh];
}

//
// shaders
//

AquaShaderHandle aqua_renderer_resource_manager_create_shader(AquaRendererResourceManager* resource_manager, const char* vertex_shader_source_path, const char* fragment_shader_source_path) {
	usize i = resource_manager_find_free_slot(resource_manager->shaders_in_use, AQUA_RENDERER_RESOURCE_MANAGER_MAX_SHADER_COUNT);
	if (i == USIZE_MAX) {
		fprintf(stderr, "[ERROR] " RESOURCE_MANAGER_LOG " Failed to add new shader, max shader count (%u) reached!\n", AQUA_RENDERER_RESOURCE_MANAGER_MAX_SHADER_COUNT);
		return AQUA_SHADER_HANDLE_INVALID;
	}

	resource_manager->shaders[i] = aqua_shader_create(vertex_shader_source_path, fragment_shader_source_path);
	if (resource_manager->shaders[i].failed) {
		fprintf(stderr, "[ERROR] " RESOURCE_MANAGER_LOG " Failed to create shader!\n");
		return AQUA_SHADER_HANDLE_INVALID;
	}
	resource_manager->shaders_in_use[i] = true;
	resource_manager->shaders_count++;

	return i;
}

void aqua_renderer_resource_manager_destroy_shader(AquaRendererResourceManager* resource_manager, AquaShaderHandle shader) {
	if (shader == AQUA_SHADER_HANDLE_INVALID) { return; }
	if (!aqua_renderer_resource_manager_is_shader_valid(resource_manager, shader)) {
		fprintf(stderr, "[ERROR] " RESOURCE_MANAGER_LOG " Tried to destroy shader %zu, which does not exist (already destroyed?)!\n", shader);
		return;
	}

	aqua_shader_destroy(&resource_manager->shaders[shader]);
	memset(&resource_manager->shaders[shader], 0, sizeof(AquaShader));
	resource_manager->shaders_in_use[shader] = false;
	resource_manager->shaders_count--;
}

//
// textures
//

// shared by every texture constructor: claims a free slot and stores the result if it succeeded
static AquaTextureHandle resource_manager_add_texture(AquaRendererResourceManager* resource_manager, AquaTexture texture) {
	if (texture.failed) {
		fprintf(stderr, "[ERROR] " RESOURCE_MANAGER_LOG " Failed to create texture!\n");
		return AQUA_TEXTURE_HANDLE_INVALID;
	}

	usize i = resource_manager_find_free_slot(resource_manager->textures_in_use, AQUA_RENDERER_RESOURCE_MANAGER_MAX_TEXTURE_COUNT);
	if (i == USIZE_MAX) {
		fprintf(stderr, "[ERROR] " RESOURCE_MANAGER_LOG " Failed to add new texture, max texture count (%u) reached!\n", AQUA_RENDERER_RESOURCE_MANAGER_MAX_TEXTURE_COUNT);
		aqua_texture_destroy(&texture);
		return AQUA_TEXTURE_HANDLE_INVALID;
	}

	resource_manager->textures[i] = texture;
	resource_manager->textures_in_use[i] = true;
	resource_manager->texture_count++;

	return i;
}

// check for space before decoding, so a full manager doesn't decode an image just to throw it away
static bool resource_manager_has_free_texture_slot(const AquaRendererResourceManager* resource_manager) {
	if (resource_manager->texture_count < AQUA_RENDERER_RESOURCE_MANAGER_MAX_TEXTURE_COUNT) { return true; }
	fprintf(stderr, "[ERROR] " RESOURCE_MANAGER_LOG " Failed to add new texture, max texture count (%u) reached!\n", AQUA_RENDERER_RESOURCE_MANAGER_MAX_TEXTURE_COUNT);
	return false;
}

AquaTextureHandle aqua_renderer_resource_manager_create_texture(AquaRendererResourceManager* resource_manager, const char* image_path) {
	if (!resource_manager_has_free_texture_slot(resource_manager)) { return AQUA_TEXTURE_HANDLE_INVALID; }
	return resource_manager_add_texture(resource_manager, aqua_texture_create(image_path));
}

AquaTextureHandle aqua_renderer_resource_manager_create_texture_from_memory(AquaRendererResourceManager* resource_manager, const u8* buffer, u32 buffer_length) {
	if (!resource_manager_has_free_texture_slot(resource_manager)) { return AQUA_TEXTURE_HANDLE_INVALID; }
	return resource_manager_add_texture(resource_manager, aqua_texture_create_from_memory(buffer, buffer_length));
}

AquaTextureHandle aqua_renderer_resource_manager_create_texture_from_pixels(AquaRendererResourceManager* resource_manager, const u8* rgba_pixels, u32 width, u32 height) {
	if (!resource_manager_has_free_texture_slot(resource_manager)) { return AQUA_TEXTURE_HANDLE_INVALID; }
	return resource_manager_add_texture(resource_manager, aqua_texture_create_from_pixels(rgba_pixels, width, height));
}

void aqua_renderer_resource_manager_destroy_texture(AquaRendererResourceManager* resource_manager, AquaTextureHandle texture) {
	if (texture == AQUA_TEXTURE_HANDLE_INVALID) { return; }
	if (!aqua_renderer_resource_manager_is_texture_valid(resource_manager, texture)) {
		fprintf(stderr, "[ERROR] " RESOURCE_MANAGER_LOG " Tried to destroy texture %zu, which does not exist (already destroyed?)!\n", texture);
		return;
	}

	aqua_texture_destroy(&resource_manager->textures[texture]);
	memset(&resource_manager->textures[texture], 0, sizeof(AquaTexture));
	resource_manager->textures_in_use[texture] = false;
	resource_manager->texture_count--;
}

//
// meshes
//

AquaMeshHandle aqua_renderer_resource_manager_create_mesh(AquaRendererResourceManager* resource_manager, AquaVertex* vertices, u32 vertices_count, GLuint* indices, u32 indices_count) {
	usize i = resource_manager_find_free_slot(resource_manager->meshes_in_use, AQUA_RENDERER_RESOURCE_MANAGER_MAX_MESH_COUNT);
	if (i == USIZE_MAX) {
		fprintf(stderr, "[ERROR] " RESOURCE_MANAGER_LOG " Failed to add new mesh, max meshes count (%u) reached!\n", AQUA_RENDERER_RESOURCE_MANAGER_MAX_MESH_COUNT);
		return AQUA_MESH_HANDLE_INVALID;
	}

	resource_manager->meshes[i] = aqua_mesh_create(vertices, vertices_count, indices, indices_count);
	if (resource_manager->meshes[i].failed) {
		fprintf(stderr, "[ERROR] " RESOURCE_MANAGER_LOG " Failed to create mesh!\n");
		return AQUA_MESH_HANDLE_INVALID;
	}
	resource_manager->meshes_in_use[i] = true;
	resource_manager->meshes_count++;

	return i;
}

void aqua_renderer_resource_manager_destroy_mesh(AquaRendererResourceManager* resource_manager, AquaMeshHandle mesh) {
	if (mesh == AQUA_MESH_HANDLE_INVALID) { return; }
	if (!aqua_renderer_resource_manager_is_mesh_valid(resource_manager, mesh)) {
		fprintf(stderr, "[ERROR] " RESOURCE_MANAGER_LOG " Tried to destroy mesh %zu, which does not exist (already destroyed?)!\n", mesh);
		return;
	}

	aqua_mesh_destroy(&resource_manager->meshes[mesh]);
	memset(&resource_manager->meshes[mesh], 0, sizeof(AquaMesh));
	resource_manager->meshes_in_use[mesh] = false;
	resource_manager->meshes_count--;
}

//
// imgui
//

void aqua_renderer_resource_manager_imgui_update(AquaRendererResourceManager* resource_manager) {
	if (resource_manager->properties.show_properties_window) {
		aqua_renderer_resource_manager_imgui_properties_window(resource_manager);
	}
}

void aqua_renderer_resource_manager_imgui_properties_window(AquaRendererResourceManager* resource_manager) {
	igBegin("Renderer Resource Manager Properties", &resource_manager->properties.show_properties_window, ImGuiWindowFlags_None);

	char separator_text_buffer[64];
	char label_buffer[64];

	snprintf(separator_text_buffer, sizeof(separator_text_buffer), "Shaders (%u/%u)", resource_manager->shaders_count, AQUA_RENDERER_RESOURCE_MANAGER_MAX_SHADER_COUNT);
	igSeparatorText(separator_text_buffer);
	for (usize i = 0; i < AQUA_RENDERER_RESOURCE_MANAGER_MAX_SHADER_COUNT; i++) {
		if (!resource_manager->shaders_in_use[i]) { continue; }
		snprintf(label_buffer, sizeof(label_buffer), "Shader %zu", i);
		aqua_shader_imgui_collapsible_header(&resource_manager->shaders[i], label_buffer);
	}

	snprintf(separator_text_buffer, sizeof(separator_text_buffer), "Textures (%u/%u)", resource_manager->texture_count, AQUA_RENDERER_RESOURCE_MANAGER_MAX_TEXTURE_COUNT);
	igSeparatorText(separator_text_buffer);
	for (usize i = 0; i < AQUA_RENDERER_RESOURCE_MANAGER_MAX_TEXTURE_COUNT; i++) {
		if (!resource_manager->textures_in_use[i]) { continue; }
		snprintf(label_buffer, sizeof(label_buffer), "Texture %zu", i);
		aqua_texture_imgui_collapsible_header(&resource_manager->textures[i], label_buffer);
	}

	snprintf(separator_text_buffer, sizeof(separator_text_buffer), "Meshes (%u/%u)", resource_manager->meshes_count, AQUA_RENDERER_RESOURCE_MANAGER_MAX_MESH_COUNT);
	igSeparatorText(separator_text_buffer);
	for (usize i = 0; i < AQUA_RENDERER_RESOURCE_MANAGER_MAX_MESH_COUNT; i++) {
		if (!resource_manager->meshes_in_use[i]) { continue; }
		snprintf(label_buffer, sizeof(label_buffer), "Mesh %zu", i);
		aqua_mesh_imgui_collapsible_header(&resource_manager->meshes[i], label_buffer);
	}

	igEnd();
}

void aqua_renderer_resource_manager_destroy(AquaRendererResourceManager* resource_manager) {
	for (usize i = 0; i < AQUA_RENDERER_RESOURCE_MANAGER_MAX_MESH_COUNT; i++) {
		if (resource_manager->meshes_in_use[i]) { aqua_renderer_resource_manager_destroy_mesh(resource_manager, i); }
	}
	for (usize i = 0; i < AQUA_RENDERER_RESOURCE_MANAGER_MAX_TEXTURE_COUNT; i++) {
		if (resource_manager->textures_in_use[i]) { aqua_renderer_resource_manager_destroy_texture(resource_manager, i); }
	}
	for (usize i = 0; i < AQUA_RENDERER_RESOURCE_MANAGER_MAX_SHADER_COUNT; i++) {
		if (resource_manager->shaders_in_use[i]) { aqua_renderer_resource_manager_destroy_shader(resource_manager, i); }
	}

	#ifndef NDEBUG
		printf("[INFO] [Aqua] [Renderer Resource Manager] Renderer Resource Manager Destroyed\n");
	#endif
}
