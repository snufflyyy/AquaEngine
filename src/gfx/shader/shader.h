#pragma once

#include <glad/glad.h>
#include <cglm/cglm.h>

#include "utils/base-types.h"

// max length of one shader stage's source, in bytes (not counting the '\0')
#define AQUA_SHADER_SOURCE_MAX_SIZE (64 * 1024)

#define AQUA_SHADER_HANDLE_INVALID USIZE_MAX

typedef GLint AquaShaderUniformLocation;

// heap-allocated, sized to the source; grows when edited in the imgui editor (up to AQUA_SHADER_SOURCE_MAX_SIZE)
typedef struct AquaShaderSource {
	char* text;
	usize capacity; // bytes allocated for text, including the '\0'
} AquaShaderSource;

typedef struct AquaShader {
	bool failed;

	GLuint program;

	AquaShaderSource vertex_source;
	AquaShaderSource fragment_source;

	AquaShaderUniformLocation projection_uniform_location;
	AquaShaderUniformLocation view_uniform_location;
	AquaShaderUniformLocation model_uniform_location;

	// material uniforms; -1 when the shader doesn't declare them (glUniform* ignores -1)
	AquaShaderUniformLocation color_uniform_location;
	AquaShaderUniformLocation color_strength_uniform_location;

	bool last_reload_failed; // shown in the debug UI
} AquaShader;

typedef usize AquaShaderHandle;

AquaShader aqua_shader_create(const char* vertex_shader_source_path, const char* fragment_shader_source_path);

// recompiles from the in-memory sources (edited in the imgui editor).
// on failure the previous program stays in use and false is returned
bool aqua_shader_reload(AquaShader* shader);

AquaShaderUniformLocation aqua_shader_get_uniform_location(const AquaShader* shader, const char* uniform_name);

void aqua_shader_set_mat4_uniform(AquaShaderUniformLocation uniform_location, mat4 value);
void aqua_shader_set_vec3_uniform(AquaShaderUniformLocation uniform_location, vec3 value);
void aqua_shader_set_vec4_uniform(AquaShaderUniformLocation uniform_location, vec4 value);
void aqua_shader_set_float_uniform(AquaShaderUniformLocation uniform_location, GLfloat value);
void aqua_shader_set_int_uniform(AquaShaderUniformLocation uniform_location, GLint value);

void aqua_shader_imgui_collapsible_header(AquaShader* shader, const char* header_label);

// deletes the program and frees the sources
void aqua_shader_destroy(AquaShader* shader);
