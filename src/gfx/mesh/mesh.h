#pragma once

#include <glad/glad.h>

#include "utils/base-types.h"
#include "gfx/vertex.h"

typedef struct AquaMesh {
	AquaVertex* vertices;
	u32 vertices_count;

	GLuint* indices;
	u32 indices_count;

	GLuint vao, vbo, ebo;
} AquaMesh;

typedef usize AquaMeshHandle;

AquaMesh aqua_mesh_create(AquaVertex* vertices, u32 vertices_count, GLuint* indices, u32 indices_count);
void aqua_mesh_reload(AquaMesh* mesh);
void aqua_mesh_destroy(AquaMesh* mesh);
