#pragma once

#include <cglm/cglm.h>

#include "gfx/material/material.h"
#include "gfx/mesh/mesh.h"
#include "utils/base-types.h"

typedef struct AquaStaticModelSubMesh {
    AquaMeshHandle mesh;
    AquaMaterialHandle material; // one of AquaStaticModel.materials, or the renderer's default material

    u32 indices_start;
    u32 indices_count;
} AquaStaticModelSubMesh;

typedef struct AquaStaticModel {
	bool failed;

	// every submesh draws from the same mesh, which the model owns
	AquaStaticModelSubMesh* submeshes;
	u32 submeshes_count;

	// one texture per glTF image; AQUA_TEXTURE_HANDLE_INVALID for images that failed to load
	AquaTextureHandle* textures;
	u32 textures_count;

	// one material per glTF material
	AquaMaterialHandle* materials;
	u32 materials_count;

	mat4 transform; // model (object-to-world) matrix used when drawing; identity after loading
} AquaStaticModel;

struct AquaRenderer;

// on failure the returned model has .failed set and is safe to draw (draws nothing) and destroy.
// the model owns its mesh, textures and materials; aqua_model_destroy releases all of them
AquaStaticModel aqua_model_create(struct AquaRenderer* renderer, const char* model_path);
void aqua_model_destroy(struct AquaRenderer* renderer, AquaStaticModel* model);

// builds model->transform as translate * rotate * scale; rotation is XYZ euler angles in degrees.
// for anything else, write model->transform directly with cglm
void aqua_model_set_transform(AquaStaticModel* model, vec3 position, vec3 rotation_degrees, vec3 scale);
