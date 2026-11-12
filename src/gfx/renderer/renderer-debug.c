#include "renderer.h"

AquaRendererResourceManagerProperties aqua_renderer_get_resource_manager_properties(const AquaRenderer* renderer) {
	return renderer->resource_manager.properties;
}

bool aqua_renderer_set_resource_manager_properties(AquaRenderer* renderer, AquaRendererResourceManagerProperties properties) {
	if (!renderer) { return false; }
	renderer->resource_manager.properties = properties;
	return true;
}

AquaMaterialManagerProperties aqua_renderer_get_material_manager_properties(const AquaRenderer* renderer) {
	return renderer->material_manager.properties;
}

bool aqua_renderer_set_material_manager_properties(AquaRenderer* renderer, AquaMaterialManagerProperties properties) {
	if (!renderer) { return false; }
	renderer->material_manager.properties = properties;
	return true;
}

AquaModelManagerProperties aqua_renderer_get_model_manager_properties(const AquaRenderer* renderer) {
	return renderer->model_manager.properties;
}

bool aqua_renderer_set_model_manager_properties(AquaRenderer* renderer, AquaModelManagerProperties properties) {
	if (!renderer) { return false; }
	renderer->model_manager.properties = properties;
	return true;
}

void aqua_renderer_imgui_properties_window(AquaRenderer* renderer) {
	AquaRendererProperties properties = aqua_renderer_get_properties(renderer);
	AquaRendererInfo info = aqua_renderer_get_info(renderer);
	AquaRendererResourceManagerProperties resources = aqua_renderer_get_resource_manager_properties(renderer);
	AquaMaterialManagerProperties materials = aqua_renderer_get_material_manager_properties(renderer);
	AquaModelManagerProperties models = aqua_renderer_get_model_manager_properties(renderer);
	igBegin("Renderer Properties", &properties.show_properties_window, ImGuiWindowFlags_None);
	igText("Video Driver: %s", info.gl_video_driver);
	igText("Vendor: %s", info.gl_vendor);
	igText("Renderer: %s", info.gl_renderer);
	igText("OpenGL Version: %s", info.gl_opengl_version);
	igText("GLSL Version: %s", info.gl_glsl_version);
	igText("Currently bound shader handle: %zu", renderer->current_shader);
	if (igButton("Rebind Shader", (ImVec2) {0.0f, 0.0f})
		&& aqua_renderer_resource_manager_is_shader_valid(&renderer->resource_manager, renderer->current_shader)) {
		glUseProgram(renderer->resource_manager.shaders[renderer->current_shader].program);
	}
	igSeparator();
	bool changed = igCheckbox("Vsync", &properties.vsync);
	changed |= igCheckbox("Wireframe", &properties.wireframe);
	changed |= igCheckbox("Rainbow Clear Color", &properties.rainbow_clear_color);
	changed |= igInputFloat("Rainbow Clear Color Speed", &properties.rainbow_clear_color_speed, 0.1f, 0.2f, "%0.2f", ImGuiInputTextFlags_None);
	changed |= igColorEdit4("Clear Color", properties.clear_color, ImGuiColorEditFlags_None);
	igSeparator();
	bool resources_changed = igCheckbox("Show Renderer Resource Manager Properties", &resources.show_properties_window);
	bool materials_changed = igCheckbox("Show Material Manager Properties", &materials.show_properties_window);
	bool models_changed = igCheckbox("Show Model Manager Properties", &models.show_properties_window);
	igEnd();
	if (changed || properties.show_properties_window != renderer->properties.show_properties_window) {
		(void) aqua_renderer_set_properties(renderer, properties);
	}
	if (resources_changed) { (void) aqua_renderer_set_resource_manager_properties(renderer, resources); }
	if (materials_changed) { (void) aqua_renderer_set_material_manager_properties(renderer, materials); }
	if (models_changed) { (void) aqua_renderer_set_model_manager_properties(renderer, models); }
}
