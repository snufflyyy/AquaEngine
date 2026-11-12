#include "renderer.h"

AquaRendererStats aqua_renderer_get_stats(const AquaRenderer* renderer) {
	return (AquaRendererStats) {
		.models_count = renderer->model_manager.count,
		.materials_count = renderer->material_manager.count,
		.meshes_count = renderer->resource_manager.meshes_count,
		.textures_count = renderer->resource_manager.textures_count,
		.shaders_count = renderer->resource_manager.shaders_count,
	};
}

AquaModelInfo aqua_renderer_get_model_info(const AquaRenderer* renderer, AquaModelHandle handle) {
	if (!aqua_model_manager_is_valid(&renderer->model_manager, handle)) { return (AquaModelInfo) {0}; }

	const AquaStaticModel* model = &renderer->model_manager.models[handle];

	return (AquaModelInfo) {
		.valid = !model->failed, .submeshes_count = model->submeshes_count,
		.materials_count = model->materials_count, .textures_count = model->textures_count,
	};
}

AquaTextureInfo aqua_renderer_get_texture_info(const AquaRenderer* renderer, AquaTextureHandle handle) {
	if (!aqua_renderer_resource_manager_is_texture_valid(&renderer->resource_manager, handle)) { return (AquaTextureInfo) {0}; }

	const AquaTexture* texture = &renderer->resource_manager.textures[handle];

	return (AquaTextureInfo) { .valid = !texture->failed, .width = texture->width, .height = texture->height };
}

AquaMeshInfo aqua_renderer_get_mesh_info(const AquaRenderer* renderer, AquaMeshHandle handle) {
	if (!aqua_renderer_resource_manager_is_mesh_valid(&renderer->resource_manager, handle)) { return (AquaMeshInfo) {0}; }

	const AquaMesh* mesh = &renderer->resource_manager.meshes[handle];

	return (AquaMeshInfo) { .valid = !mesh->failed, .vertices_count = mesh->vertices_count, .indices_count = mesh->indices_count };
}

bool aqua_renderer_is_material_valid(const AquaRenderer* renderer, AquaMaterialHandle material) {
	return aqua_material_manager_is_valid(&renderer->material_manager, material);
}

AquaMaterialProperties aqua_renderer_get_material_properties(const AquaRenderer* renderer, AquaMaterialHandle material) {
	if (!aqua_renderer_is_material_valid(renderer, material)) { return AQUA_MATERIAL_DEFAULT_PROPERTIES; }
	return aqua_material_get_properties(&renderer->material_manager.materials[material]);
}

bool aqua_renderer_set_material_properties(AquaRenderer* renderer, AquaMaterialHandle material, AquaMaterialProperties properties) {
	if (properties.shader == AQUA_SHADER_HANDLE_INVALID) { properties.shader = renderer->base_material_shader; }
	return aqua_material_manager_set_properties(&renderer->material_manager, &renderer->resource_manager, material, properties);
}

bool aqua_renderer_reload_shader(AquaRenderer* renderer, AquaShaderHandle shader) {
	if (!aqua_renderer_resource_manager_is_shader_valid(&renderer->resource_manager, shader)) { return false; }

	AquaShader* resource = &renderer->resource_manager.shaders[shader];

	if (!aqua_shader_reload(resource)) { return false; }
	if (renderer->current_shader == shader) { glUseProgram(resource->program); }

	return true;
}
