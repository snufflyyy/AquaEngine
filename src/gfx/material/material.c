#include "material.h"
#include "gfx/shader/shader.h"

#include <stdio.h>
#include <assert.h>

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include <cimgui.h>
#include <cimgui_impl.h>

#include "gfx/renderer/renderer.h"
#include "gfx/texture/texture.h"

AquaMaterial* aqua_material_create(AquaRenderer* renderer, AquaTextureHandle texture, AquaColor color) {
    AquaMaterial* material = (AquaMaterial*) malloc(sizeof(AquaMaterial));
    if (!material) {
        fprintf(stderr, "[ERROR] [Aqua] [Material] Failed to allocate memory for material!\n");
        return NULL;
    }

    material->shader = renderer->base_material_shader;
    material->texture = texture;

    glm_vec4_copy(color, material->color);
    material->color_uniform_location = aqua_shader_get_uniform_location(renderer->resource_manager.shaders[material->shader], "color");

    material->color_strength = 0.0f;
    material->color_strength_uniform_location = aqua_shader_get_uniform_location(renderer->resource_manager.shaders[material->shader], "color_strength");

    material->properties = (AquaMaterialProperties) { .show_properties_window = false };

    return material;
}

void aqua_material_change_shader(AquaRenderer* renderer, AquaMaterial* material, AquaShaderHandle shader) {
    material->shader = shader;
    material->color_uniform_location = aqua_shader_get_uniform_location(renderer->resource_manager.shaders[material->shader], "color");
    material->color_strength_uniform_location = aqua_shader_get_uniform_location(renderer->resource_manager.shaders[material->shader], "color_strength");
}

void aqua_material_imgui_properties_window(AquaMaterial* material) {
	igBegin("Material Properties", &material->properties.show_properties_window, ImGuiWindowFlags_None);
	igText("Shader Handle: %u", material->shader);
	igText("Texture Handle: %u", material->texture);
	igColorEdit4("Color", material->color, ImGuiColorEditFlags_None);
	igSliderFloat("Color Strength", &material->color_strength, 0.0f, 1.0f, "%0.2f", ImGuiSliderFlags_None);
	igEnd();
}

void aqua_material_imgui_update(AquaMaterial* material) {
	if (material->properties.show_properties_window) {
		aqua_material_imgui_properties_window(material);
	}
}

void aqua_material_destroy(AquaMaterial* material) {
    assert(material != NULL);

    free(material);
}
