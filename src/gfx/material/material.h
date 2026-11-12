#pragma once

#include "gfx/shader/shader.h"
#include "gfx/texture/texture.h"
#include <aqua/types.h>

#include <aqua/renderer.h>

typedef struct AquaMaterial {
    AquaShaderHandle shader;
    AquaTextureHandle texture; // AQUA_TEXTURE_HANDLE_INVALID binds the renderer's white default texture

    AquaColor color;
    f32 color_strength; // 0 = texture only, 1 = color only
} AquaMaterial;

AquaMaterialProperties aqua_material_get_properties(const AquaMaterial* material);

// texture may be NULL (no preview is drawn)
bool aqua_material_imgui_collapsible_header(AquaMaterialProperties* properties, const AquaTexture* texture, const char* header_label);
