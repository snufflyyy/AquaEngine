#pragma once

#include <cglm/cglm.h>

#include "gfx/renderer/renderer.h"
#include "gfx/material/material.h"
#include "gfx/mesh/mesh.h"
#include "utils/base-types.h"

typedef struct AquaSubMesh {
    AquaMeshHandle mesh;
    AquaMaterial* material;

    u32 indices_start;
    u32 indices_count;
} AquaSubMesh;

typedef struct AquaModel {
	AquaSubMesh* submeshes;
	u32 submeshes_count;

	mat4 transform;
} AquaModel;

AquaModel aqua_model_create(AquaRenderer* renderer, const char* model_path);
void aqua_model_destroy(AquaModel* model);
