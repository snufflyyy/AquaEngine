#pragma once

#include <glad/glad.h>
#include <cglm/cglm.h>

#include <aqua/types.h>

// max length of one shader stage's source, in bytes (not counting the '\0')
#define AQUA_SHADER_SOURCE_MAX_SIZE (64 * 1024) // 64 kb

typedef GLint AquaShaderUniformLocation;
#define AQUA_SHADER_UNIFORM_LOCATION_INVALID (-1)

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

	// Material uniforms use AQUA_SHADER_UNIFORM_LOCATION_INVALID when absent.
	AquaShaderUniformLocation color_uniform_location;
	AquaShaderUniformLocation color_strength_uniform_location;

	bool last_reload_failed; // shown in the debug UI
} AquaShader;

AquaShader aqua_shader_create(const char* vertex_shader_source_path, const char* fragment_shader_source_path);

// recompiles from the in-memory sources (edited in the imgui editor).
// on failure the previous program stays in use and false is returned
bool aqua_shader_reload(AquaShader* shader);

// Returns AQUA_SHADER_UNIFORM_LOCATION_INVALID for missing or optimized-out uniforms.
AquaShaderUniformLocation aqua_shader_get_uniform_location(const AquaShader* shader, const char* uniform_name);

void aqua_shader_set_mat4_uniform(AquaShaderUniformLocation uniform_location, mat4 value);
void aqua_shader_set_vec3_uniform(AquaShaderUniformLocation uniform_location, vec3 value);
void aqua_shader_set_vec4_uniform(AquaShaderUniformLocation uniform_location, vec4 value);
void aqua_shader_set_float_uniform(AquaShaderUniformLocation uniform_location, GLfloat value);
void aqua_shader_set_int_uniform(AquaShaderUniformLocation uniform_location, GLint value);

void aqua_shader_imgui_collapsible_header(AquaShader* shader, const char* header_label);

// deletes the program and frees the sources
void aqua_shader_destroy(AquaShader* shader);
