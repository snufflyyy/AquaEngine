#pragma once

#include <cglm/cglm.h>

#include "gfx/material/material.h"
#include "gfx/mesh/mesh.h"
#include "utils/base-types.h"

typedef struct AquaSubMesh {
    AquaMeshHandle mesh;
    AquaMaterial* material;

    u32 indices_start;
    u32 indices_count;
} AquaStaticModelSubMesh;

typedef struct AquaStaticModel {
	AquaStaticModelSubMesh* submeshes;
	u32 submeshes_count;

	mat4 transform;
} AquaStaticModel;

struct AquaRenderer;

AquaStaticModel aqua_model_create(struct AquaRenderer* renderer, const char* model_path);
void aqua_model_destroy(AquaStaticModel* model);
