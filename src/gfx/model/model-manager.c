#include "model-manager.h"

#include <stdio.h>
#include <string.h>

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include <cimgui.h>

void aqua_model_manager_init(AquaModelManager* model_manager) {
	memset(model_manager, 0, sizeof(*model_manager));

	#ifndef NDEBUG
		printf("[INFO] [Aqua] [Model Manager] Created Model Manager\n");
	#endif
}

AquaModelHandle aqua_model_manager_create(AquaModelManager* model_manager, struct AquaRenderer* renderer, const char* model_path) {
	if (!model_path || model_path[0] == '\0') {
		fprintf(stderr, "[ERROR] [Aqua] [Model Manager] Model path is empty!\n");
		return AQUA_MODEL_HANDLE_INVALID;
	}

	// Check capacity before loading, so a full manager creates no asset resources.
	for (usize i = 0; i < AQUA_MODEL_MANAGER_MAX_MODEL_COUNT; i++) {
		if (model_manager->in_use[i]) { continue; }

		AquaStaticModel model = aqua_model_create(renderer, model_path);
		// The loader has already cleaned up partial resources on failure.
		if (model.failed) { return AQUA_MODEL_HANDLE_INVALID; }

		model_manager->models[i] = model;
		model_manager->in_use[i] = true;
		model_manager->count++;
		return i;
	}

	fprintf(stderr, "[ERROR] [Aqua] [Model Manager] Failed to create model, max model count (%u) reached!\n", AQUA_MODEL_MANAGER_MAX_MODEL_COUNT);
	return AQUA_MODEL_HANDLE_INVALID;
}

bool aqua_model_manager_is_valid(const AquaModelManager* model_manager, AquaModelHandle model) {
	return model < AQUA_MODEL_MANAGER_MAX_MODEL_COUNT && model_manager->in_use[model];
}

AquaStaticModel* aqua_model_manager_get(AquaModelManager* model_manager, AquaModelHandle model) {
	if (!aqua_model_manager_is_valid(model_manager, model)) { return NULL; }
	return &model_manager->models[model];
}

void aqua_model_manager_destroy_model(AquaModelManager* model_manager, struct AquaRenderer* renderer, AquaModelHandle model) {
	if (model == AQUA_MODEL_HANDLE_INVALID) { return; }
	if (!aqua_model_manager_is_valid(model_manager, model)) {
		fprintf(stderr, "[ERROR] [Aqua] [Model Manager] Tried to destroy model %zu, which does not exist (already destroyed?)!\n", model);
		return;
	}

	aqua_model_destroy(renderer, &model_manager->models[model]);
	model_manager->in_use[model] = false;
	model_manager->count--;
}

void aqua_model_manager_imgui_update(AquaModelManager* model_manager) {
	if (model_manager->properties.show_properties_window) {
		aqua_model_manager_imgui_properties_window(model_manager);
	}
}

void aqua_model_manager_imgui_properties_window(AquaModelManager* model_manager) {
	AquaModelManagerProperties properties = model_manager->properties;
	igBegin("Model Manager Properties", &properties.show_properties_window, ImGuiWindowFlags_None);

	char separator_text_buffer[64];
	snprintf(separator_text_buffer, sizeof(separator_text_buffer), "Models (%u/%u)", model_manager->count, AQUA_MODEL_MANAGER_MAX_MODEL_COUNT);
	igSeparatorText(separator_text_buffer);

	for (usize i = 0; i < AQUA_MODEL_MANAGER_MAX_MODEL_COUNT; i++) {
		if (!model_manager->in_use[i]) { continue; }
		const AquaStaticModel* model = &model_manager->models[i];
		igText("Model %zu: %u submeshes, %u materials, %u textures", i, model->submeshes_count, model->materials_count, model->textures_count);
	}

	igEnd();
	model_manager->properties = properties;
}

void aqua_model_manager_destroy(AquaModelManager* model_manager, struct AquaRenderer* renderer) {
	for (usize i = 0; i < AQUA_MODEL_MANAGER_MAX_MODEL_COUNT; i++) {
		if (model_manager->in_use[i]) { aqua_model_manager_destroy_model(model_manager, renderer, i); }
	}
	memset(model_manager, 0, sizeof(*model_manager));

	#ifndef NDEBUG
		printf("[INFO] [Aqua] [Model Manager] Model Manager Destroyed\n");
	#endif
}
