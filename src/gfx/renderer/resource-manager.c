#include "resource-manager.h"
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
		return USIZE_MAX;
	}

	usize i = resource_manager->shaders_count;

	resource_manager->shaders[i] = aqua_shader_create(vertex_shader_source_path, fragment_shader_source_path);
	resource_manager->shaders_count++;

	return i;
}

AquaMeshHandle aqua_renderer_resource_manager_create_mesh(AquaRendererResourceManager* resource_manager, AquaVertex* vertices, u32 vertices_count, GLuint* indices, u32 indices_count) {
	if (resource_manager->meshes_count >= AQUA_RENDERER_RESOURCE_MANAGER_MAX_MESH_COUNT) {
		fprintf(stderr, "[ERROR] [Aqua] [Renderer Resource Manager] Failed to add new mesh, max meshes count (%u) reached!\n", AQUA_RENDERER_RESOURCE_MANAGER_MAX_MESH_COUNT);
		return USIZE_MAX;
	}

	usize i = resource_manager->meshes_count;

	resource_manager->meshes[i] = aqua_mesh_create(vertices, vertices_count, indices, indices_count);
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
	igSeparatorText("Shaders");
	for (usize i = 0; i < resource_manager->shaders_count; i++) {
		char label_buffer[64];
		snprintf(label_buffer, sizeof(label_buffer), "Shader %u", (u32) i);

		if (igCollapsingHeader_BoolPtr(label_buffer, NULL, ImGuiTreeNodeFlags_None)) {
			AquaShader* shader = &resource_manager->shaders[i];

			igText("Program ID: %u", shader->program);
			igText("Projection Uniform Location: %d", shader->projection_uniform_location);
			igText("View Uniform Location: %d", shader->view_uniform_location);
			igText("Model Uniform Location: %d", shader->model_uniform_location);
			igSeparator();
			igInputTextMultiline("Vertex Shader Source", shader->vertex_shader_source, AQUA_SHADER_SOURCE_BUFFER_SIZE, (ImVec2) { 0.0f, 0.0f }, ImGuiInputTextFlags_None, 0, NULL);
			igInputTextMultiline("Fragment Shader Source", shader->fragment_shader_source, AQUA_SHADER_SOURCE_BUFFER_SIZE, (ImVec2) { 0.0f, 0.0f }, ImGuiInputTextFlags_None, 0, NULL);
			igSeparator();
			igText("NOTE: If you are reloading the currently bound shader,\nyou need to rebind it in the renderer properties window.");
			if (igButton("Reload", (ImVec2) { 0.0f, 0.0f })) {
				aqua_shader_reload(shader);
			}
			igSeparator();
		}
	}
	igSeparatorText("Meshes");
	for (usize i = 0; i < resource_manager->meshes_count; i++) {
		char label_buffer[64];
		snprintf(label_buffer, sizeof(label_buffer), "Mesh %u", (u32) i);

		if (igCollapsingHeader_BoolPtr(label_buffer, NULL, ImGuiTreeNodeFlags_None)) {
			AquaMesh* mesh = &resource_manager->meshes[i];
			igText("Indices Count: %u\n", (u32) mesh->indices_count);
			igText("VAO: %u\n", (u32) mesh->vao);
			igText("VBO: %u\n", (u32) mesh->vbo);
			igText("EBO: %u\n", (u32) mesh->ebo);
		}
	}
	igEnd();
}

void aqua_renderer_resource_manager_destroy(AquaRendererResourceManager* resource_manager) {
	for (usize i = 0; i < resource_manager->meshes_count; i++) {
		aqua_mesh_destroy(&resource_manager->meshes[i]);
	}
	for (usize i = 0; i < resource_manager->shaders_count; i++) {
		aqua_shader_destroy(resource_manager->shaders[i]);
	}
}
