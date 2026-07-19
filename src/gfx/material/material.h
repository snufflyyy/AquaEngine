#pragma once

#include "gfx/shader/shader.h"
#include "gfx/color.h"
#include "gfx/texture/texture.h"

typedef struct AquaMaterialProperties {
	bool show_properties_window;
} AquaMaterialProperties;

typedef struct AquaMaterial {
    AquaShaderHandle shader;

    AquaTextureHandle texture;

    AquaColor color;
    f32 color_strength;
    AquaShaderUniformLocation color_uniform_location;
    AquaShaderUniformLocation color_strength_uniform_location;

    AquaMaterialProperties properties;
} AquaMaterial;

struct AquaRenderer;

AquaMaterial* aqua_material_create(struct AquaRenderer* renderer, AquaTextureHandle texture, AquaColor color);
void aqua_material_imgui_update(AquaMaterial* material);
void aqua_material_imgui_properties_window(AquaMaterial* material);
void aqua_material_destroy(AquaMaterial* material);
