#pragma once

#include "gfx/model/model.h"
#include <aqua/types.h>
#include <aqua/debug.h>

#define AQUA_MODEL_MANAGER_MAX_MODEL_COUNT 1024

// Fixed slots, matching the material manager. Destroyed slots are reused, so stale
// handles can refer to replacement models. Detach all instances before unloading.
typedef struct AquaModelManager {
	AquaStaticModel models[AQUA_MODEL_MANAGER_MAX_MODEL_COUNT];
	bool in_use[AQUA_MODEL_MANAGER_MAX_MODEL_COUNT];
	u32 count; // live models

	AquaModelManagerProperties properties;
} AquaModelManager;

struct AquaRenderer;

void aqua_model_manager_init(AquaModelManager* model_manager);

// renderer must own this manager; loading and cleanup require its graphics context.
AquaModelHandle aqua_model_manager_create(AquaModelManager* model_manager, struct AquaRenderer* renderer, const char* model_path);
void aqua_model_manager_destroy_model(AquaModelManager* model_manager, struct AquaRenderer* renderer, AquaModelHandle model);

// Borrowed pointer, valid until destruction. Do not copy or destroy the owned asset.
// NULL for invalid or destroyed handles.
AquaStaticModel* aqua_model_manager_get(AquaModelManager* model_manager, AquaModelHandle model);
bool aqua_model_manager_is_valid(const AquaModelManager* model_manager, AquaModelHandle model);

void aqua_model_manager_imgui_update(AquaModelManager* model_manager);
void aqua_model_manager_imgui_properties_window(AquaModelManager* model_manager);

// Release models before the renderer's material and GPU resource managers.
void aqua_model_manager_destroy(AquaModelManager* model_manager, struct AquaRenderer* renderer);
