#include "resource-manager.h"
#include "gfx/texture/texture.h"
#include "utils/base-types.h"

#include <stdio.h>

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include <cimgui.h>
#include <cimgui_impl.h>

#include "gfx/shader/shader.h"
#include "gfx/mesh/mesh.h"

AquaRendererResourceManager aqua_renderer_resource_manager_create(void) {
	return (AquaRendererResourceManager) {0};
}

AquaShaderHandle aqua_renderer_resource_manager_create_shader(AquaRendererResourceManager* resource_manager, const char* vertex_shader_source_path, const char* fragment_shader_source_path) {
	if (resource_manager->shaders_count >= AQUA_RENDERER_RESOURCE_MANAGER_MAX_SHADER_COUNT) {
		fprintf(stderr, "[ERROR] [Aqua] [Renderer Resource Manager] Failed to add new shader, max shader count (%u) reached!\n", AQUA_RENDERER_RESOURCE_MANAGER_MAX_SHADER_COUNT);
		return AQUA_SHADER_HANDLE_INVALID;
	}

	usize i = resource_manager->shaders_count;

	resource_manager->shaders[i] = aqua_shader_create(vertex_shader_source_path, fragment_shader_source_path);
	if (resource_manager->shaders[i].failed) {
		fprintf(stderr, "[ERROR] [Aqua] [Renderer Resource Manager] Failed to create shader!\n");
		return AQUA_SHADER_HANDLE_INVALID;
	}
	resource_manager->shaders_count++;

	return i;
}

AquaTextureHandle aqua_renderer_resource_manager_create_texture(AquaRendererResourceManager* resource_manager, const char* image_path) {
	if (resource_manager->texture_count >= AQUA_RENDERER_RESOURCE_MANAGER_MAX_TEXTURE_COUNT) {
		fprintf(stderr, "[ERROR] [Aqua] [Renderer Resource Manager] Failed to add new texture, max texture count (%u) reached!\n", AQUA_RENDERER_RESOURCE_MANAGER_MAX_SHADER_COUNT);
		return AQUA_SHADER_HANDLE_INVALID;
	}

	usize i = resource_manager->texture_count;

	resource_manager->textures[i] = aqua_texture_create(image_path);
	if (resource_manager->shaders[i].failed) {
		fprintf(stderr, "[ERROR] [Aqua] [Renderer Resource Manager] Failed to create shader!\n");
		return AQUA_SHADER_HANDLE_INVALID;
	}
	resource_manager->texture_count++;

	return i;
}

AquaMeshHandle aqua_renderer_resource_manager_create_mesh(AquaRendererResourceManager* resource_manager, AquaVertex* vertices, u32 vertices_count, GLuint* indices, u32 indices_count) {
	if (resource_manager->meshes_count >= AQUA_RENDERER_RESOURCE_MANAGER_MAX_MESH_COUNT) {
		fprintf(stderr, "[ERROR] [Aqua] [Renderer Resource Manager] Failed to add new mesh, max meshes count (%u) reached!\n", AQUA_RENDERER_RESOURCE_MANAGER_MAX_MESH_COUNT);
		return AQUA_MESH_HANDLE_INVALID;
	}

	usize i = resource_manager->meshes_count;

	resource_manager->meshes[i] = aqua_mesh_create(vertices, vertices_count, indices, indices_count);
	if (resource_manager->meshes[i].failed) {
		fprintf(stderr, "[ERROR] [Aqua] [Renderer Resource Manager] Failed to create mesh!\n");
		return AQUA_MESH_HANDLE_INVALID;
	}
	resource_manager->meshes_count++;

	return i;
}

void aqua_renderer_resource_manager_imgui_update(AquaRendererResourceManager* resource_manager) {
	if (resource_manager->properties.show_properties_window) {
		aqua_renderer_resource_manager_imgui_properties_window(resource_manager);
	}
}

void aqua_renderer_resource_manager_imgui_properties_window(AquaRendererResourceManager* resource_manager) {
	igBegin("Renderer Resource Manager Properties", &resource_manager->properties.show_properties_window, ImGuiWindowFlags_None);

	char separator_text_buffer[64];

	snprintf(separator_text_buffer, sizeof(separator_text_buffer), "Shaders (%u/%u)", resource_manager->shaders_count, AQUA_RENDERER_RESOURCE_MANAGER_MAX_SHADER_COUNT);
	igSeparatorText(separator_text_buffer);
	for (usize i = 0; i < resource_manager->shaders_count; i++) {
		AquaShader* shader = &resource_manager->shaders[i];

		char label_buffer[64];
		snprintf(label_buffer, sizeof(label_buffer), "Shader %u", (u32) i);

		aqua_shader_imgui_collapsible_header(shader, label_buffer);
	}

	snprintf(separator_text_buffer, sizeof(separator_text_buffer), "Textures (%u/%u)", resource_manager->texture_count, AQUA_RENDERER_RESOURCE_MANAGER_MAX_TEXTURE_COUNT);
	igSeparatorText(separator_text_buffer);
	for (usize i = 0; i < resource_manager->texture_count; i++) {
		AquaTexture* texture = &resource_manager->textures[i];

		char label_buffer[64];
		snprintf(label_buffer, sizeof(label_buffer), "Texture %u", (u32) i);

		aqua_texture_imgui_collapsible_header(texture, label_buffer);
	}

	snprintf(separator_text_buffer, sizeof(separator_text_buffer), "Meshes (%u/%u)", resource_manager->meshes_count, AQUA_RENDERER_RESOURCE_MANAGER_MAX_MESH_COUNT);
	igSeparatorText(separator_text_buffer);
	for (usize i = 0; i < resource_manager->meshes_count; i++) {
		AquaMesh* mesh = &resource_manager->meshes[i];

		char label_buffer[64];
		snprintf(label_buffer, sizeof(label_buffer), "Mesh %u", (u32) i);

		aqua_mesh_imgui_collapsible_header(mesh, label_buffer);
	}
	igEnd();
}

void aqua_renderer_resource_manager_destroy(AquaRendererResourceManager* resource_manager) {
	for (usize i = 0; i < resource_manager->meshes_count; i++) {
		aqua_mesh_destroy(&resource_manager->meshes[i]);
	}
	for (usize i = 0; i < resource_manager->texture_count; i++) {
		aqua_texture_destroy(&resource_manager->textures[i]);
	}
	for (usize i = 0; i < resource_manager->shaders_count; i++) {
		aqua_shader_destroy(resource_manager->shaders[i]);
	}
}
