#pragma once

#include "gfx/material/material.h"
#include "gfx/renderer/resource-manager.h"
#include <aqua/types.h>
#include <aqua/debug.h>
#include <aqua/renderer.h>

#define AQUA_MATERIAL_MANAGER_MAX_MATERIAL_COUNT 1024

// Fixed slots, so an AquaMaterial* from aqua_material_manager_get stays valid until that material is destroyed.
// Destroyed slots are reused: a handle kept after destroying its material may later refer to a different one.
typedef struct AquaMaterialManager {
	AquaMaterial materials[AQUA_MATERIAL_MANAGER_MAX_MATERIAL_COUNT];
	bool in_use[AQUA_MATERIAL_MANAGER_MAX_MATERIAL_COUNT];
	u32 count; // live materials

	AquaMaterialManagerProperties properties;
} AquaMaterialManager;

void aqua_material_manager_init(AquaMaterialManager* material_manager);

// Invalid inputs or full capacity return INVALID without claiming a slot.
AquaMaterialHandle aqua_material_manager_create(AquaMaterialManager* material_manager, const AquaRendererResourceManager* resources, AquaShaderHandle shader, AquaTextureHandle texture, AquaColor color);
void aqua_material_manager_destroy_material(AquaMaterialManager* material_manager, AquaMaterialHandle material);

// NULL for invalid or destroyed handles
AquaMaterial* aqua_material_manager_get(AquaMaterialManager* material_manager, AquaMaterialHandle material);
bool aqua_material_manager_is_valid(const AquaMaterialManager* material_manager, AquaMaterialHandle material);
bool aqua_material_manager_set_properties(AquaMaterialManager* material_manager, const AquaRendererResourceManager* resources, AquaMaterialHandle material, AquaMaterialProperties properties);

// the resource manager is only read, for texture previews
void aqua_material_manager_imgui_update(AquaMaterialManager* material_manager, const AquaRendererResourceManager* resource_manager);
void aqua_material_manager_imgui_properties_window(AquaMaterialManager* material_manager, const AquaRendererResourceManager* resource_manager);

void aqua_material_manager_destroy(AquaMaterialManager* material_manager);
