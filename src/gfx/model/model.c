#include "model.h"
#include "cglm/mat4.h"
#include "cglm/vec3.h"
#include "gfx/color.h"
#include "gfx/material/material.h"
#include "gfx/mesh/mesh.h"
#include "gfx/renderer/renderer.h"
#include "gfx/texture/texture.h"
#include "gfx/vertex.h"
#include "utils/base-types.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CGLTF_IMPLEMENTATION
#include "cgltf.h"

static const cgltf_accessor* model_find_attribute(const cgltf_primitive* primitive, cgltf_attribute_type type) {
	for (cgltf_size i = 0; i < primitive->attributes_count; i++) {
		if (primitive->attributes[i].type == type) {
			return primitive->attributes[i].data;
		}
	}

	return NULL;
}

// both passes over the primitives must agree on what gets loaded, so the rule lives here
static bool model_primitive_is_drawable(const cgltf_primitive* primitive) {
	return primitive->type == cgltf_primitive_type_triangles
		&& model_find_attribute(primitive, cgltf_attribute_type_position) != NULL;
}

static AquaTextureHandle model_load_image(AquaRenderer* renderer, const cgltf_image* image, const char* model_path) {
	// .glb and embedded images
	if (image->buffer_view) {
		const u8* buffer = cgltf_buffer_view_data(image->buffer_view);
		if (!buffer || image->buffer_view->size > UINT32_MAX) {
			fprintf(stderr, "[ERROR] [Aqua] [Model] Image buffer view is missing or too large in '%s'!\n", model_path);
			return AQUA_TEXTURE_HANDLE_INVALID;
		}
		return aqua_renderer_create_texture_from_memory(renderer, buffer, (u32) image->buffer_view->size);
	}

	// .gltf with an image file next to it; the uri is relative to the model file
	if (image->uri && strncmp(image->uri, "data:", 5) != 0) {
		const char* last_slash = strrchr(model_path, '/');
		const char* last_backslash = strrchr(model_path, '\\');
		if (last_backslash > last_slash) { last_slash = last_backslash; }

		usize directory_length = last_slash ? (usize) (last_slash - model_path) + 1 : 0;
		usize uri_length = strlen(image->uri);

		char* image_path = (char*) malloc(directory_length + uri_length + 1);
		if (!image_path) {
			fprintf(stderr, "[ERROR] [Aqua] [Model] Failed to allocate memory for image path!\n");
			return AQUA_TEXTURE_HANDLE_INVALID;
		}
		memcpy(image_path, model_path, directory_length);
		memcpy(image_path + directory_length, image->uri, uri_length + 1);
		cgltf_decode_uri(image_path + directory_length);

		AquaTextureHandle texture = aqua_renderer_create_texture(renderer, image_path);
		free(image_path);
		return texture;
	}

	fprintf(stderr, "[WARNING] [Aqua] [Model] Skipping unsupported image (data URI) in '%s'\n", model_path);
	return AQUA_TEXTURE_HANDLE_INVALID;
}

static AquaMaterialHandle model_create_material(AquaRenderer* renderer, const cgltf_data* data, const cgltf_material* gltf_material, const AquaTextureHandle* textures) {
	AquaTextureHandle texture = AQUA_TEXTURE_HANDLE_INVALID;
	AquaColor color = { 1.0f, 1.0f, 1.0f, 1.0f };

	if (gltf_material->has_pbr_metallic_roughness) {
		const cgltf_pbr_metallic_roughness* pbr = &gltf_material->pbr_metallic_roughness;
		// not glm_vec4_copy: that does an aligned SIMD load and cgltf's float[4] is not 16-byte aligned
		for (int c = 0; c < 4; c++) { color[c] = pbr->base_color_factor[c]; }

		if (pbr->base_color_texture.texture && pbr->base_color_texture.texture->image) {
			texture = textures[cgltf_image_index(data, pbr->base_color_texture.texture->image)];
		}
	}

	AquaMaterialHandle material = aqua_renderer_create_material(renderer, texture, color);
	if (material != AQUA_MATERIAL_HANDLE_INVALID && texture == AQUA_TEXTURE_HANDLE_INVALID) {
		// untextured: the shader samples the white default texture, so show the base color instead
		aqua_renderer_get_material(renderer, material)->color_strength = 1.0f;
	}

	return material;
}

AquaStaticModel aqua_model_create(AquaRenderer* renderer, const char* model_path) {
	AquaStaticModel model = {
		.failed = true,
		.transform = GLM_MAT4_IDENTITY_INIT,
	};

	cgltf_data* data = NULL;
	AquaVertex* vertices = NULL;
	u32* indices = NULL;

	cgltf_options options = {0};
	cgltf_result result = cgltf_parse_file(&options, model_path, &data);
	if (result != cgltf_result_success) {
		fprintf(stderr, "[ERROR] [Aqua] [Model] Failed to parse gltf file '%s' (cgltf error %d)!\n", model_path, (int) result);
		goto cleanup;
	}

	result = cgltf_load_buffers(&options, data, model_path);
	if (result != cgltf_result_success) {
		fprintf(stderr, "[ERROR] [Aqua] [Model] Failed to load buffers for '%s' (cgltf error %d)!\n", model_path, (int) result);
		goto cleanup;
	}

	// checks accessor/buffer bounds, attribute counts and index ranges so the reads below stay in bounds
	result = cgltf_validate(data);
	if (result != cgltf_result_success) {
		fprintf(stderr, "[ERROR] [Aqua] [Model] Invalid gltf file '%s' (cgltf error %d)!\n", model_path, (int) result);
		goto cleanup;
	}

	// textures
	if (data->images_count > 0) {
		model.textures = (AquaTextureHandle*) malloc(sizeof(AquaTextureHandle) * data->images_count);
		if (!model.textures) {
			fprintf(stderr, "[ERROR] [Aqua] [Model] Failed to allocate memory for texture handles!\n");
			goto cleanup;
		}
		model.textures_count = (u32) data->images_count;
		for (u32 i = 0; i < model.textures_count; i++) {
			// a failed image stays invalid and its materials draw white
			model.textures[i] = model_load_image(renderer, &data->images[i], model_path);
		}
	}

	// materials, one per gltf material; primitives without one use the renderer's default material
	if (data->materials_count > 0) {
		model.materials = (AquaMaterialHandle*) malloc(sizeof(AquaMaterialHandle) * data->materials_count);
		if (!model.materials) {
			fprintf(stderr, "[ERROR] [Aqua] [Model] Failed to allocate memory for materials!\n");
			goto cleanup;
		}
		// not calloc: 0 is a valid handle, and cleanup destroys every non-invalid entry
		model.materials_count = (u32) data->materials_count;
		for (u32 i = 0; i < model.materials_count; i++) { model.materials[i] = AQUA_MATERIAL_HANDLE_INVALID; }

		for (u32 i = 0; i < model.materials_count; i++) {
			model.materials[i] = model_create_material(renderer, data, &data->materials[i], model.textures);
			if (model.materials[i] == AQUA_MATERIAL_HANDLE_INVALID) { goto cleanup; }
		}
	}

	// count what we are going to load
	usize total_vertices_count = 0;
	usize total_indices_count = 0;
	usize total_submeshes_count = 0;
	bool skipped_primitives = false;

	for (cgltf_size n = 0; n < data->nodes_count; n++) {
		const cgltf_mesh* mesh = data->nodes[n].mesh;
		if (!mesh) { continue; }

		for (cgltf_size p = 0; p < mesh->primitives_count; p++) {
			const cgltf_primitive* primitive = &mesh->primitives[p];
			if (!model_primitive_is_drawable(primitive)) {
				skipped_primitives = true;
				continue;
			}

			const cgltf_accessor* position_accessor = model_find_attribute(primitive, cgltf_attribute_type_position);
			if (!position_accessor) { continue; } // already ruled out by model_primitive_is_drawable; keeps the optimizer happy
			cgltf_size primitive_vertices_count = position_accessor->count;
			total_vertices_count += primitive_vertices_count;
			total_indices_count += primitive->indices ? primitive->indices->count : primitive_vertices_count;
			total_submeshes_count++;
		}
	}

	if (skipped_primitives) {
		fprintf(stderr, "[WARNING] [Aqua] [Model] Skipped non-triangle or position-less primitives in '%s'\n", model_path);
	}
	if (total_submeshes_count == 0) {
		fprintf(stderr, "[ERROR] [Aqua] [Model] '%s' has no drawable triangle primitives!\n", model_path);
		goto cleanup;
	}
	if (total_vertices_count > UINT32_MAX || total_indices_count > UINT32_MAX) {
		fprintf(stderr, "[ERROR] [Aqua] [Model] '%s' is too large for 32-bit indices!\n", model_path);
		goto cleanup;
	}

	vertices = (AquaVertex*) malloc(sizeof(AquaVertex) * total_vertices_count);
	indices = (u32*) malloc(sizeof(u32) * total_indices_count);
	model.submeshes = (AquaStaticModelSubMesh*) calloc(total_submeshes_count, sizeof(AquaStaticModelSubMesh));
	if (!vertices || !indices || !model.submeshes) {
		fprintf(stderr, "[ERROR] [Aqua] [Model] Failed to allocate memory for '%s' geometry!\n", model_path);
		goto cleanup;
	}

	// fill, baking each node's world transform into its vertices
	u32 vertex_offset = 0;
	u32 index_offset = 0;
	u32 submesh_index = 0;

	for (cgltf_size n = 0; n < data->nodes_count; n++) {
		const cgltf_node* node = &data->nodes[n];
		if (!node->mesh) { continue; }

		mat4 world_matrix;
		cgltf_node_transform_world(node, (cgltf_float*) world_matrix);

		const cgltf_mesh* mesh = node->mesh;
		for (cgltf_size p = 0; p < mesh->primitives_count; p++) {
			const cgltf_primitive* primitive = &mesh->primitives[p];
			if (!model_primitive_is_drawable(primitive)) { continue; }

			const cgltf_accessor* position_accessor = model_find_attribute(primitive, cgltf_attribute_type_position);
			const cgltf_accessor* normal_accessor = model_find_attribute(primitive, cgltf_attribute_type_normal);
			const cgltf_accessor* texture_coords_accessor = model_find_attribute(primitive, cgltf_attribute_type_texcoord);

			u32 primitive_vertices_count = (u32) position_accessor->count;

			for (u32 i = 0; i < primitive_vertices_count; i++) {
				AquaVertex* vertex = &vertices[vertex_offset + i];

				vec3 local_position = { 0.0f, 0.0f, 0.0f };
				cgltf_accessor_read_float(position_accessor, i, local_position, 3);
				glm_mat4_mulv3(world_matrix, local_position, 1.0f, vertex->position);

				vec3 local_normal = { 0.0f, 1.0f, 0.0f };
				if (normal_accessor) { cgltf_accessor_read_float(normal_accessor, i, local_normal, 3); }
				glm_mat4_mulv3(world_matrix, local_normal, 0.0f, vertex->normal);
				glm_vec3_normalize(vertex->normal);

				vertex->texture_coords[0] = 0.0f;
				vertex->texture_coords[1] = 0.0f;
				if (texture_coords_accessor) { cgltf_accessor_read_float(texture_coords_accessor, i, vertex->texture_coords, 2); }
			}

			u32 primitive_indices_count;
			if (primitive->indices) {
				primitive_indices_count = (u32) primitive->indices->count;
				for (u32 i = 0; i < primitive_indices_count; i++) {
					indices[index_offset + i] = (u32) cgltf_accessor_read_index(primitive->indices, i) + vertex_offset;
				}
			} else {
				// non-indexed primitive: every three vertices are a triangle
				primitive_indices_count = primitive_vertices_count;
				for (u32 i = 0; i < primitive_indices_count; i++) {
					indices[index_offset + i] = i + vertex_offset;
				}
			}

			AquaStaticModelSubMesh* submesh = &model.submeshes[submesh_index];
			submesh->indices_start = index_offset;
			submesh->indices_count = primitive_indices_count;
			submesh->material = primitive->material
				? model.materials[cgltf_material_index(data, primitive->material)]
				: renderer->default_material;

			vertex_offset += primitive_vertices_count;
			index_offset += primitive_indices_count;
			submesh_index++;
		}
	}
	AquaMeshHandle mesh = aqua_renderer_create_mesh(renderer, vertices, (u32) total_vertices_count, indices, (u32) total_indices_count);
	if (mesh == AQUA_MESH_HANDLE_INVALID) {
		fprintf(stderr, "[ERROR] [Aqua] [Model] Failed to create mesh for '%s'!\n", model_path);
		goto cleanup;
	}

	// only set once the mesh exists: aqua_model_destroy frees submeshes[0].mesh when submeshes_count > 0
	model.submeshes_count = submesh_index;
	for (u32 i = 0; i < model.submeshes_count; i++) {
		model.submeshes[i].mesh = mesh;
	}

	model.failed = false;

	#ifndef NDEBUG
		printf("[INFO] [Aqua] [Model] Loaded Model '%s' (%u submeshes, %u materials, %u textures)\n", model_path, model.submeshes_count, model.materials_count, model.textures_count);
	#endif

cleanup:
	// CPU-side copies only; the GPU mesh and textures belong to the model now
	free(indices);
	free(vertices);
	if (data) { cgltf_free(data); }

	if (model.failed) {
		aqua_model_destroy(renderer, &model);
		model.failed = true;
	}

	return model;
}

void aqua_model_destroy(AquaRenderer* renderer, AquaStaticModel* model) {
	if (!model) { return; }

	// only a loaded model has submeshes; failed loads and repeat destroys stay quiet
	bool was_loaded = model->submeshes_count > 0;

	// materials first: they reference the textures
	for (u32 i = 0; i < model->materials_count; i++) {
		if (model->materials[i] != AQUA_MATERIAL_HANDLE_INVALID) { aqua_renderer_destroy_material(renderer, model->materials[i]); }
	}
	for (u32 i = 0; i < model->textures_count; i++) {
		if (model->textures[i] != AQUA_TEXTURE_HANDLE_INVALID) { aqua_renderer_destroy_texture(renderer, model->textures[i]); }
	}
	if (model->submeshes_count > 0) {
		aqua_renderer_destroy_mesh(renderer, model->submeshes[0].mesh);
	}

	free(model->materials);
	free(model->textures);
	free(model->submeshes);

	model->materials = NULL;
	model->materials_count = 0;
	model->textures = NULL;
	model->textures_count = 0;
	model->submeshes = NULL;
	model->submeshes_count = 0;

	#ifndef NDEBUG
		if (was_loaded) { printf("[INFO] [Aqua] [Model] Model Destroyed\n"); }
	#else
		(void) was_loaded;
	#endif
}

void aqua_model_set_transform(AquaStaticModel* model, vec3 position, vec3 rotation_degrees, vec3 scale) {
	vec3 rotation_radians = {
		glm_rad(rotation_degrees[0]),
		glm_rad(rotation_degrees[1]),
		glm_rad(rotation_degrees[2]),
	};

	mat4 rotation;
	glm_euler_xyz(rotation_radians, rotation);

	glm_translate_make(model->transform, position);
	glm_mat4_mul(model->transform, rotation, model->transform);
	glm_scale(model->transform, scale);
}
