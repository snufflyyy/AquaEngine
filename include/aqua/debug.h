#pragma once

#include "types.h"

// Editable debug settings without exposing manager storage or ImGui backend types.
typedef struct AquaRendererResourceManagerProperties { bool show_properties_window; } AquaRendererResourceManagerProperties;
typedef struct AquaMaterialManagerProperties { bool show_properties_window; } AquaMaterialManagerProperties;
typedef struct AquaModelManagerProperties { bool show_properties_window; } AquaModelManagerProperties;

AquaRendererResourceManagerProperties aqua_renderer_get_resource_manager_properties(const AquaRenderer* renderer);
bool aqua_renderer_set_resource_manager_properties(AquaRenderer* renderer, AquaRendererResourceManagerProperties properties);
AquaMaterialManagerProperties aqua_renderer_get_material_manager_properties(const AquaRenderer* renderer);
bool aqua_renderer_set_material_manager_properties(AquaRenderer* renderer, AquaMaterialManagerProperties properties);
AquaModelManagerProperties aqua_renderer_get_model_manager_properties(const AquaRenderer* renderer);
bool aqua_renderer_set_model_manager_properties(AquaRenderer* renderer, AquaModelManagerProperties properties);

// Call camera debug UI during the project's imgui_update callback.
// Custom cimgui UI can explicitly link AquaEngineDebug for its headers/dependency.
void aqua_camera_imgui_update(AquaCamera* camera);
