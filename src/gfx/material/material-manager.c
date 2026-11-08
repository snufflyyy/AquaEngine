#include "material-manager.h"

#include <stdio.h>
#include <string.h>

#include <cglm/cglm.h>

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include <cimgui.h>
#include <cimgui_impl.h>

void aqua_material_manager_init(AquaMaterialManager* material_manager) {
	memset(material_manager, 0, sizeof(*material_manager));

	#ifndef NDEBUG
		printf("[INFO] [Aqua] [Material Manager] Created Material Manager\n");
	#endif
}

AquaMaterialHandle aqua_material_manager_create(AquaMaterialManager* material_manager, AquaShaderHandle shader, AquaTextureHandle texture, AquaColor color) {
	// first free slot, so slots released by destroyed materials get reused
	for (usize i = 0; i < AQUA_MATERIAL_MANAGER_MAX_MATERIAL_COUNT; i++) {
		if (material_manager->in_use[i]) { continue; }

		AquaMaterial* material = &material_manager->materials[i];
		material->shader = shader;
		material->texture = texture;
		glm_vec4_copy(color, material->color);
		material->color_strength = 0.0f;

		material_manager->in_use[i] = true;
		material_manager->count++;

		return i;
	}

	fprintf(stderr, "[ERROR] [Aqua] [Material Manager] Failed to create material, max material count (%u) reached!\n", AQUA_MATERIAL_MANAGER_MAX_MATERIAL_COUNT);
	return AQUA_MATERIAL_HANDLE_INVALID;
}

bool aqua_material_manager_is_valid(const AquaMaterialManager* material_manager, AquaMaterialHandle material) {
	return material < AQUA_MATERIAL_MANAGER_MAX_MATERIAL_COUNT && material_manager->in_use[material];
}

AquaMaterial* aqua_material_manager_get(AquaMaterialManager* material_manager, AquaMaterialHandle material) {
	if (!aqua_material_manager_is_valid(material_manager, material)) { return NULL; }
	return &material_manager->materials[material];
}

void aqua_material_manager_destroy_material(AquaMaterialManager* material_manager, AquaMaterialHandle material) {
	if (material == AQUA_MATERIAL_HANDLE_INVALID) { return; }
	if (!aqua_material_manager_is_valid(material_manager, material)) {
		fprintf(stderr, "[ERROR] [Aqua] [Material Manager] Tried to destroy material %zu, which does not exist (already destroyed?)!\n", material);
		return;
	}

	material_manager->in_use[material] = false;
	memset(&material_manager->materials[material], 0, sizeof(AquaMaterial));
	material_manager->count--;
}

void aqua_material_manager_imgui_update(AquaMaterialManager* material_manager, const AquaRendererResourceManager* resource_manager) {
	if (material_manager->properties.show_properties_window) {
		aqua_material_manager_imgui_properties_window(material_manager, resource_manager);
	}
}

void aqua_material_manager_imgui_properties_window(AquaMaterialManager* material_manager, const AquaRendererResourceManager* resource_manager) {
	igBegin("Material Manager Properties", &material_manager->properties.show_properties_window, ImGuiWindowFlags_None);

	char separator_text_buffer[64];
	snprintf(separator_text_buffer, sizeof(separator_text_buffer), "Materials (%u/%u)", material_manager->count, AQUA_MATERIAL_MANAGER_MAX_MATERIAL_COUNT);
	igSeparatorText(separator_text_buffer);

	for (usize i = 0; i < AQUA_MATERIAL_MANAGER_MAX_MATERIAL_COUNT; i++) {
		if (!material_manager->in_use[i]) { continue; }
		AquaMaterial* material = &material_manager->materials[i];

		const AquaTexture* texture = NULL;
		if (aqua_renderer_resource_manager_is_texture_valid(resource_manager, material->texture)) {
			texture = &resource_manager->textures[material->texture];
		}

		char label_buffer[64];
		snprintf(label_buffer, sizeof(label_buffer), "Material %zu", i);
		aqua_material_imgui_collapsible_header(material, texture, label_buffer);
	}

	igEnd();
}

void aqua_material_manager_destroy(AquaMaterialManager* material_manager) {
	// materials only reference shaders and textures, which the resource manager owns, so there is nothing to release
	memset(material_manager, 0, sizeof(*material_manager));

	#ifndef NDEBUG
		printf("[INFO] [Aqua] [Material Manager] Material Manager Destroyed\n");
	#endif
}
