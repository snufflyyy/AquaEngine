#pragma once

#include "gfx/shader/shader.h"
#include "gfx/color.h"
#include "gfx/texture/texture.h"
#include "utils/base-types.h"

// index into the renderer's material manager; materials are created and owned by the engine
typedef usize AquaMaterialHandle;
#define AQUA_MATERIAL_HANDLE_INVALID USIZE_MAX

typedef struct AquaMaterial {
    AquaShaderHandle shader;
    AquaTextureHandle texture; // AQUA_TEXTURE_HANDLE_INVALID binds the renderer's white default texture

    AquaColor color;
    f32 color_strength; // 0 = texture only, 1 = color only
} AquaMaterial;

// texture may be NULL (no preview is drawn)
void aqua_material_imgui_collapsible_header(AquaMaterial* material, const AquaTexture* texture, const char* header_label);
