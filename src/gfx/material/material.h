#pragma once

#include "gfx/shader/shader.h"
#include "gfx/color.h"


typedef struct AquaMaterial {
    AquaShaderHandle shader;

    AquaColor color;
    AquaShaderUniformLocation color_uniform_location;
} AquaMaterial;

struct AquaRenderer;

AquaMaterial* aqua_material_create(struct AquaRenderer* renderer, AquaShaderHandle shader, AquaColor color);
void aqua_material_destroy(AquaMaterial* material);
