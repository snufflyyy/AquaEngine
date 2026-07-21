#include "model.h"
#include "cglm/mat4.h"
#include "cglm/vec3.h"
#include "gfx/mesh/mesh.h"
#include "gfx/renderer/renderer.h"
#include "gfx/vertex.h"
#include "utils/base-types.h"

#define CGLTF_IMPLEMENTATION
#include "cgltf.h"

static cgltf_accessor* aqua_cgltf_find_attribute(cgltf_primitive* primitive, cgltf_attribute_type type) {
   	for (cgltf_size i = 0; i < primitive->attributes_count; i++) {
          if (primitive->attributes[i].type == type) {
              return primitive->attributes[i].data;
          }
      }

      return NULL;
}

AquaModel aqua_model_create(AquaRenderer* renderer ,const char* model_path) {
    cgltf_options options = {0};
    cgltf_data* data = NULL;
    if (cgltf_parse_file(&options, model_path, &data) != cgltf_result_success) {
        fprintf(stderr, "[ERROR] [Aqua] [Model] Failed to parse gltf file!\n");
    }

    cgltf_load_buffers(&options, data, model_path);

    u32 total_vertices_count = 0;
    u32 total_indices_count = 0;
    u32 total_primitive_count = 0;

    for (usize n = 0; n < data->nodes_count; n++) {
        cgltf_node* node = &data->nodes[n];
        if (!node->mesh) { continue; }

        cgltf_mesh* mesh = data->nodes[n].mesh;
        total_primitive_count += mesh->primitives_count;

        for (usize p = 0; p < mesh->primitives_count; p++) {
            cgltf_primitive* primitive = &mesh->primitives[p];

            cgltf_accessor* position_accessor = aqua_cgltf_find_attribute(primitive, cgltf_attribute_type_position);
            if (!position_accessor) {
                // error
            }

            total_vertices_count += position_accessor->count;

            cgltf_accessor* indices_accessor = primitive->indices;
            if (!indices_accessor) {
                // error
            }

            total_indices_count += indices_accessor->count;
        }
    }

    AquaVertex* aqua_mesh_vertices = (AquaVertex*) malloc(sizeof(AquaVertex) * total_vertices_count);
    if (!aqua_mesh_vertices) {
        // error
    }

    u32* aqua_mesh_indices = (u32*) malloc(sizeof(u32) * total_indices_count);
    if (!aqua_mesh_indices) {
        // error
    }

    AquaSubMesh* submeshes = (AquaSubMesh*) malloc(sizeof(AquaSubMesh) * total_primitive_count);
    if (!submeshes) {
        // error
    }

    u32 current_vertex_offset = 0;
    u32 current_indices_offset = 0;
    u32 current_submesh_index = 0;

    for (usize n = 0; n < data->nodes_count; n++) {
        cgltf_node* node = &data->nodes[n];
        if (!node->mesh) { continue; }

        mat4 world_matrix;
        cgltf_node_transform_world(node, (cgltf_float*) world_matrix);

        cgltf_mesh* mesh = node->mesh;
        for (usize p = 0; p < mesh->primitives_count; p++) {
            cgltf_primitive* primitive = &mesh->primitives[p];

            cgltf_accessor* position_accessor = aqua_cgltf_find_attribute(primitive, cgltf_attribute_type_position);
            if (!position_accessor) {
                // error
            }
            cgltf_accessor* normal_accessor = aqua_cgltf_find_attribute(primitive, cgltf_attribute_type_normal);
            if (!normal_accessor) {
                // error
            }
            cgltf_accessor* texture_coords_accessor = aqua_cgltf_find_attribute(primitive, cgltf_attribute_type_texcoord);
            if (!texture_coords_accessor) {
                // error
            }

            for (usize i = 0; i < position_accessor->count; i++) {
                AquaVertex* vertex = &aqua_mesh_vertices[current_vertex_offset + i];

                vec3 local_position;
                cgltf_accessor_read_float(position_accessor, i, local_position, 3);
                glm_mat4_mulv3(world_matrix, local_position, 1.0f, vertex->position);

                vec3 local_normal;
                cgltf_accessor_read_float(normal_accessor, i, local_normal, 3);
                glm_mat4_mulv3(world_matrix, local_normal, 0.0f, vertex->normal);
                glm_vec3_normalize(vertex->normal);

                cgltf_accessor_read_float(texture_coords_accessor, i, vertex->texture_coords, 2);
            }

            cgltf_accessor* indices_accessor = primitive->indices;
            if (!indices_accessor) {
                // error
            }

            for (usize i = 0; i < indices_accessor->count; i++) {
                aqua_mesh_indices[current_indices_offset + i] = (u32) cgltf_accessor_read_index(indices_accessor, i) + current_vertex_offset;
            }

            submeshes[current_submesh_index].indices_start = current_indices_offset;
            submeshes[current_submesh_index].indices_count = (u32) indices_accessor->count;
            // handle submesh material

            current_vertex_offset += position_accessor->count;
            current_indices_offset += indices_accessor->count;
            current_submesh_index++;
        }
    }

    AquaMeshHandle mesh = aqua_renderer_create_mesh(renderer, aqua_mesh_vertices, total_vertices_count, aqua_mesh_indices, total_indices_count);

    AquaModel model = {
        .submeshes = submeshes,
        .submeshes_count = total_primitive_count,
        .transform = GLM_MAT4_IDENTITY_INIT,
    };

    for (usize i = 0; i < total_primitive_count; i++) {
        submeshes[i].mesh = mesh;
    }

    free(aqua_mesh_indices);
    free(aqua_mesh_vertices);
	cgltf_free(data);

	return model;
}

void aqua_model_destroy(AquaModel* model) {

}
