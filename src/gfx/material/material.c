#include "material.h"

#include <stdio.h>
#include <assert.h>

#include "gfx/renderer/renderer.h"
#include "gfx/shader/shader.h"

AquaMaterial* aqua_material_create(AquaRenderer* renderer, AquaShaderHandle shader, AquaColor color) {
    AquaMaterial* material = (AquaMaterial*) malloc(sizeof(AquaMaterial));
    if (!material) {
        fprintf(stderr, "[ERROR] [Aqua] [Material] Failed to allocate memory for material!\n");
        return NULL;
    }

    material->shader = shader;
    glm_vec3_copy(color, material->color);
    material->color_uniform_location = aqua_shader_get_uniform_location(renderer->resource_manager.shaders[shader], "color");

    return material;
}

void aqua_material_destroy(AquaMaterial* material) {
    assert(material != NULL);

    free(material);
}
