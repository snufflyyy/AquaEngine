#pragma once

#include <glad/glad.h>

#include <aqua/types.h>

typedef struct AquaMesh {
	bool failed;

	AquaVertex* vertices;
	u32 vertices_count;

	GLuint* indices;
	u32 indices_count;

	GLuint vao, vbo, ebo;
} AquaMesh;

AquaMesh aqua_mesh_create(const AquaVertex* vertices, u32 vertices_count, const GLuint* indices, u32 indices_count);
void aqua_mesh_reload(AquaMesh* mesh);

void aqua_mesh_imgui_collapsible_header(AquaMesh* mesh, const char* header_label);

void aqua_mesh_destroy(AquaMesh* mesh);
