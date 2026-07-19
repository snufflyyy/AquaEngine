#pragma once

#include <glad/glad.h>
#include <cglm/cglm.h>

#include "utils/base-types.h"

#define AQUA_SHADER_SOURCE_BUFFER_SIZE 2048

#define AQUA_SHADER_HANDLE_INVALID USIZE_MAX

typedef GLint AquaShaderUniformLocation;

typedef struct AquaShader {
	bool failed;

	GLuint program;

	char vertex_shader_source[AQUA_SHADER_SOURCE_BUFFER_SIZE];
	char fragment_shader_source[AQUA_SHADER_SOURCE_BUFFER_SIZE];

	AquaShaderUniformLocation projection_uniform_location;
	AquaShaderUniformLocation view_uniform_location;
	AquaShaderUniformLocation model_uniform_location;
} AquaShader;

typedef usize AquaShaderHandle;

AquaShader aqua_shader_create(const char* vertex_shader_source_path, const char* fragment_shader_source_path);

void aqua_shader_reload(AquaShader* shader);

AquaShaderUniformLocation aqua_shader_get_uniform_location(AquaShader shader, const char* uniform_name);

void aqua_shader_set_mat4_uniform(AquaShaderUniformLocation uniform_location, mat4 value);
void aqua_shader_set_vec3_uniform(AquaShaderUniformLocation uniform_location, vec3 value);
void aqua_shader_set_vec4_uniform(AquaShaderUniformLocation uniform_location, vec4 value);
void aqua_shader_set_float_uniform(AquaShaderUniformLocation uniform_location, GLfloat value);
void aqua_shader_set_int_uniform(AquaShaderUniformLocation uniform_location, GLint value);

void aqua_shader_imgui_collapsible_header(AquaShader* shader, const char* header_label);

void aqua_shader_destroy(AquaShader shader);
