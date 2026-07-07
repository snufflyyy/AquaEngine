#include "gfx/mesh/mesh.h"
#include "gfx/vertex.h"

#include <glad/glad.h>

#include "utils/base-types.h"

AquaMesh aqua_mesh_create(AquaVertex* vertices, u32 vertices_count, GLuint* indices, u32 indices_count) {
	AquaMesh mesh = {
		.vertices = (AquaVertex*) malloc(sizeof(AquaVertex) * vertices_count),
		.vertices_count = vertices_count,
		.indices = (GLuint*) malloc(sizeof(GLuint) * indices_count),
		.indices_count = indices_count,
	};

	if (!mesh.vertices) {
		fprintf(stderr, "[ERROR] [Aqua] [Mesh] Failed to allocate memory for mesh's vertices!\n");
		return (AquaMesh) {0};
	}

	if (!mesh.indices) {
		fprintf(stderr, "[ERROR] [Aqua] [Mesh] Failed to allocate memory for mesh's indices!\n");
		free(mesh.vertices);
		return (AquaMesh) {0};
	}

    glGenVertexArrays(1, &mesh.vao);
    glGenBuffers(1, &mesh.vbo);
    glGenBuffers(1, &mesh.ebo);

    glBindVertexArray(mesh.vao);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);

    glBufferData(GL_ARRAY_BUFFER, (u32) vertices_count * sizeof(AquaVertex), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, (u32) indices_count * sizeof(GLuint), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(AquaVertex), (void*) 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(AquaVertex), (void*) offsetof(AquaVertex, normal));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(AquaVertex), (void*) offsetof(AquaVertex, texture_coords));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    return mesh;
}

void aqua_mesh_reload(AquaMesh* mesh) {
	glBindVertexArray(mesh->vao);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);

    glBufferData(GL_ARRAY_BUFFER, mesh->vertices_count * sizeof(AquaVertex), mesh->vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->indices_count * sizeof(GLuint), mesh->vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(AquaVertex), (void*) 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(AquaVertex), (void*) offsetof(AquaVertex, normal));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(AquaVertex), (void*) offsetof(AquaVertex, texture_coords));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

void aqua_mesh_destroy(AquaMesh* mesh) {
	if (!mesh) { return; }

	glDeleteVertexArrays(1, &mesh->vao);
	glDeleteBuffers(1, &mesh->vbo);
	glDeleteBuffers(1, &mesh->ebo);

	free(mesh->indices);
	mesh->indices_count = 0;
	free(mesh->vertices);
	mesh->vertices_count = 0;
}
