#pragma once

#include <glad/glad.h>

#include "utils/base-types.h"
#include "gfx/vertex.h"

#define AQUA_MESH_HANDLE_INVALID USIZE_MAX

typedef struct AquaMesh {
	bool failed;

	AquaVertex* vertices;
	u32 vertices_count;

	GLuint* indices;
	u32 indices_count;

	GLuint vao, vbo, ebo;
} AquaMesh;

typedef usize AquaMeshHandle;

AquaMesh aqua_mesh_create(AquaVertex* vertices, u32 vertices_count, GLuint* indices, u32 indices_count);
void aqua_mesh_reload(AquaMesh* mesh);

void aqua_mesh_imgui_collapsible_header(AquaMesh* mesh, const char* header_label);

void aqua_mesh_destroy(AquaMesh* mesh);
