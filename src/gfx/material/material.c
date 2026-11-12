#include "material.h"

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include <cimgui.h>
#include <cimgui_impl.h>

AquaMaterialProperties aqua_material_get_properties(const AquaMaterial* material) {
	AquaMaterialProperties properties = {
		.shader = material->shader, .texture = material->texture,
		.color_strength = material->color_strength,
	};
	for (int i = 0; i < 4; i++) { properties.color[i] = material->color[i]; }
	return properties;
}

bool aqua_material_imgui_collapsible_header(AquaMaterialProperties* properties, const AquaTexture* texture, const char* header_label) {
	bool changed = false;
	if (igCollapsingHeader_BoolPtr(header_label, NULL, ImGuiTreeNodeFlags_None)) {
		igPushID_Str(header_label);

		if (properties->shader == AQUA_SHADER_HANDLE_INVALID) { igText("Shader Handle: none"); }
		else { igText("Shader Handle: %zu", properties->shader); }

		if (properties->texture == AQUA_TEXTURE_HANDLE_INVALID) { igText("Texture Handle: none (default white)"); }
		else { igText("Texture Handle: %zu", properties->texture); }

		changed |= igColorEdit4("Color", properties->color, ImGuiColorEditFlags_None);
		changed |= igSliderFloat("Color Strength", &properties->color_strength, 0.0f, 1.0f, "%0.2f", ImGuiSliderFlags_None);

		if (texture && !texture->failed && texture->width > 0 && texture->height > 0) {
			const f32 preview_size = 64.0f;
			f32 aspect = (f32) texture->width / (f32) texture->height;
			ImVec2 size = (aspect >= 1.0f)
				? (ImVec2) { preview_size, preview_size / aspect }
				: (ImVec2) { preview_size * aspect, preview_size };
			igImage((ImTextureRef) { NULL, texture->id }, size, (ImVec2) { 0.0f, 0.0f }, (ImVec2) { 1.0f, 1.0f });
		}

		igPopID();
		igSeparator();
	}
	return changed;
}
