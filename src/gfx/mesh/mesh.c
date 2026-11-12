#include "gfx/mesh/mesh.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glad/glad.h>

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include <cimgui.h>
#include <cimgui_impl.h>

#include <aqua/types.h>

AquaMesh aqua_mesh_create(const AquaVertex* vertices, u32 vertices_count, const GLuint* indices, u32 indices_count) {
	AquaMesh mesh = {
		.vertices = (AquaVertex*) malloc(sizeof(AquaVertex) * vertices_count),
		.vertices_count = vertices_count,
		.indices = (GLuint*) malloc(sizeof(GLuint) * indices_count),
		.indices_count = indices_count,
	};

	if (!mesh.vertices) {
		fprintf(stderr, "[ERROR] [Aqua] [Mesh] Failed to allocate memory for mesh's vertices!\n");
		free(mesh.indices); // allocated alongside the vertices, may have succeeded
		return (AquaMesh) { .failed = true };
	}

	memcpy(mesh.vertices, vertices, vertices_count * sizeof(AquaVertex));

	if (!mesh.indices) {
		fprintf(stderr, "[ERROR] [Aqua] [Mesh] Failed to allocate memory for mesh's indices!\n");
		free(mesh.vertices);
		return (AquaMesh) { .failed = true };
	}

	memcpy(mesh.indices, indices, indices_count * sizeof(GLuint));

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

    mesh.failed = false;

    return mesh;
}

void aqua_mesh_reload(AquaMesh* mesh) {
	if (mesh->failed) { return; }

	glBindVertexArray(mesh->vao);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);

    glBufferData(GL_ARRAY_BUFFER, mesh->vertices_count * sizeof(AquaVertex), mesh->vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->indices_count * sizeof(GLuint), mesh->indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(AquaVertex), (void*) 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(AquaVertex), (void*) offsetof(AquaVertex, normal));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(AquaVertex), (void*) offsetof(AquaVertex, texture_coords));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

void aqua_mesh_imgui_collapsible_header(AquaMesh* mesh, const char* header_label) {
	if (igCollapsingHeader_BoolPtr(header_label, NULL, ImGuiTreeNodeFlags_None)) {
		igPushID_Ptr(mesh);
		igText("Vertices Count %u", (u32) mesh->vertices_count);

		// big models have hundreds of thousands of vertices, so they get their own scrolling region
		// and the clipper only builds widgets for the rows that are actually visible
		igBeginChild_Str("##vertices", (ImVec2) { 0.0f, 300.0f }, ImGuiChildFlags_Borders, ImGuiWindowFlags_None);
		ImGuiListClipper* clipper = ImGuiListClipper_ImGuiListClipper();
		ImGuiListClipper_Begin(clipper, (int) mesh->vertices_count, -1.0f); // -1: measured from the first vertex
		while (ImGuiListClipper_Step(clipper)) {
			for (int j = clipper->DisplayStart; j < clipper->DisplayEnd; j++) {
				igPushID_Int(j);
				igText("Vertex %d", j);
				igInputFloat3("Position", mesh->vertices[j].position, "%0.2f", ImGuiInputTextFlags_None);
				igInputFloat3("Normal", mesh->vertices[j].normal, "%0.2f", ImGuiInputTextFlags_None);
				igInputFloat2("Texture Coordinate", mesh->vertices[j].texture_coords, "%0.2f", ImGuiInputTextFlags_None);
				igPopID();
			}
		}
		ImGuiListClipper_End(clipper);
		ImGuiListClipper_destroy(clipper);
		igEndChild();

		igSeparator();
		igText("Indices Count: %u", (u32) mesh->indices_count);
		igSeparator();
		igText("VAO: %u", (u32) mesh->vao);
		igText("VBO: %u", (u32) mesh->vbo);
		igText("EBO: %u", (u32) mesh->ebo);
		igSeparator();
		if (igButton("Reload Mesh", (ImVec2) { 0.0f, 0.0f })) {
			aqua_mesh_reload(mesh);
		}
		igPopID();
		igSeparator();
	}
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
