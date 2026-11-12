#pragma once

#include "gfx/material/material.h"
#include "gfx/mesh/mesh.h"
#include <aqua/types.h>

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
} AquaStaticModel;

struct AquaRenderer;

// Low-level loader/cleanup used by the model manager. Applications use the renderer's
// model wrappers instead. Do not copy or directly destroy a manager-owned model.
// On failure .failed is set and partial resources are already released.
// The model owns its mesh, textures and materials; aqua_model_destroy releases them.
AquaStaticModel aqua_model_create(struct AquaRenderer* renderer, const char* model_path);
void aqua_model_destroy(struct AquaRenderer* renderer, AquaStaticModel* model);
