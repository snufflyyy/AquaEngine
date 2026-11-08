#include "shader.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glad/glad.h>

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include <cimgui.h>
#include <cimgui_impl.h>

#include "utils/file/file.h"

static const char* shader_stage_name(GLenum shader_type) {
	switch (shader_type) {
		case GL_VERTEX_SHADER: return "vertex";
		case GL_FRAGMENT_SHADER: return "fragment";
		default: return "unknown";
	}
}

static bool shader_source_load(AquaShaderSource* source, const char* path, GLenum shader_type) {
	char* text = aqua_file_to_string(path);
	if (!text) {
		fprintf(stderr, "[ERROR] [Aqua] [Shader] Failed to load %s shader source '%s'!\n", shader_stage_name(shader_type), path);
		return false;
	}

	usize length = strlen(text);
	if (length > AQUA_SHADER_SOURCE_MAX_SIZE) {
		fprintf(stderr, "[ERROR] [Aqua] [Shader] %s shader source '%s' is %zu bytes, the limit is %d!\n", shader_stage_name(shader_type), path, length, AQUA_SHADER_SOURCE_MAX_SIZE);
		aqua_file_free_string(text);
		return false;
	}

	source->text = text;
	source->capacity = length + 1;
	return true;
}

static void shader_source_free(AquaShaderSource* source) {
	aqua_file_free_string(source->text);
	source->text = NULL;
	source->capacity = 0;
}

static bool shader_create_module(GLuint* result, const char* shader_source, GLenum shader_type) {
	GLuint shader_module = glCreateShader(shader_type);
	if (shader_module == 0) {
		fprintf(stderr, "[ERROR] [Aqua] [Shader] Failed to create OpenGL %s shader module!\n", shader_stage_name(shader_type));
		return false;
	}
	glShaderSource(shader_module, 1, &shader_source, NULL);
	glCompileShader(shader_module);

	GLint shader_compile_result;
	glGetShaderiv(shader_module, GL_COMPILE_STATUS, &shader_compile_result);
	if (shader_compile_result == GL_FALSE) {
		char info_log[1024];
		glGetShaderInfoLog(shader_module, sizeof(info_log), NULL, info_log);
		fprintf(stderr, "[ERROR] [Aqua] [Shader] Failed to compile %s shader:\n%s\n", shader_stage_name(shader_type), info_log);

		glDeleteShader(shader_module);
		return false;
	}

	*result = shader_module;
	return true;
}

// compiles and links a complete program; on failure nothing is left behind and *result is untouched
static bool shader_build_program(GLuint* result, const char* vertex_shader_source, const char* fragment_shader_source) {
	GLuint vertex_shader;
	if (!shader_create_module(&vertex_shader, vertex_shader_source, GL_VERTEX_SHADER)) { return false; }

	GLuint fragment_shader;
	if (!shader_create_module(&fragment_shader, fragment_shader_source, GL_FRAGMENT_SHADER)) {
		glDeleteShader(vertex_shader);
		return false;
	}

	GLuint program = glCreateProgram();
	if (program == 0) {
		fprintf(stderr, "[ERROR] [Aqua] [Shader] Failed to create OpenGL shader program!\n");
		glDeleteShader(vertex_shader);
		glDeleteShader(fragment_shader);
		return false;
	}
	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);
	glLinkProgram(program);

	// the program keeps what it needs once linked
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	GLint link_status;
	glGetProgramiv(program, GL_LINK_STATUS, &link_status);
	if (link_status == GL_FALSE) {
		char info_log[1024];
		glGetProgramInfoLog(program, sizeof(info_log), NULL, info_log);
		fprintf(stderr, "[ERROR] [Aqua] [Shader] Failed to link shader program:\n%s\n", info_log);
		glDeleteProgram(program);
		return false;
	}

	*result = program;
	return true;
}

static void shader_fetch_uniform_locations(AquaShader* shader) {
	shader->projection_uniform_location = aqua_shader_get_uniform_location(shader, "projection");
	shader->view_uniform_location = aqua_shader_get_uniform_location(shader, "view");
	shader->model_uniform_location = aqua_shader_get_uniform_location(shader, "model");
	shader->color_uniform_location = aqua_shader_get_uniform_location(shader, "color");
	shader->color_strength_uniform_location = aqua_shader_get_uniform_location(shader, "color_strength");
}

AquaShader aqua_shader_create(const char* vertex_shader_source_path, const char* fragment_shader_source_path) {
	AquaShader shader = { .failed = true };

	if (!shader_source_load(&shader.vertex_source, vertex_shader_source_path, GL_VERTEX_SHADER)) { goto fail; }
	if (!shader_source_load(&shader.fragment_source, fragment_shader_source_path, GL_FRAGMENT_SHADER)) { goto fail; }

	if (!shader_build_program(&shader.program, shader.vertex_source.text, shader.fragment_source.text)) {
		fprintf(stderr, "[ERROR] [Aqua] [Shader] Failed to build shader from '%s' and '%s'!\n", vertex_shader_source_path, fragment_shader_source_path);
		goto fail;
	}

	shader_fetch_uniform_locations(&shader);
	shader.failed = false;
	return shader;

fail:
	shader_source_free(&shader.vertex_source);
	shader_source_free(&shader.fragment_source);
	return (AquaShader) { .failed = true };
}

bool aqua_shader_reload(AquaShader* shader) {
	if (shader->failed) { return false; }

	// build the replacement first, so a broken edit leaves the working program in place
	GLuint new_program;
	if (!shader_build_program(&new_program, shader->vertex_source.text, shader->fragment_source.text)) {
		fprintf(stderr, "[ERROR] [Aqua] [Shader] Reload of program %u failed, keeping the previous program!\n", shader->program);
		return false;
	}

	// if the old program is bound, GL defers deleting it until it's unbound;
	// the renderer binds the new one the next time a material using this shader is bound
	glDeleteProgram(shader->program);
	shader->program = new_program;
	shader_fetch_uniform_locations(shader);

	return true;
}

AquaShaderUniformLocation aqua_shader_get_uniform_location(const AquaShader* shader, const char* uniform_name) {
	return glGetUniformLocation(shader->program, uniform_name);
}

void aqua_shader_set_mat4_uniform(AquaShaderUniformLocation uniform_location, mat4 value) {
	glUniformMatrix4fv(uniform_location, 1, GL_FALSE, *value);
}

void aqua_shader_set_vec3_uniform(AquaShaderUniformLocation uniform_location, vec3 value) {
	glUniform3fv(uniform_location, 1, value);
}

void aqua_shader_set_vec4_uniform(AquaShaderUniformLocation uniform_location, vec4 value) {
	glUniform4fv(uniform_location, 1, value);
}

void aqua_shader_set_float_uniform(AquaShaderUniformLocation uniform_location, GLfloat value) {
	glUniform1f(uniform_location, value);
}

void aqua_shader_set_int_uniform(AquaShaderUniformLocation uniform_location, GLint value) {
	glUniform1i(uniform_location, value);
}

// lets the editor grow the source buffer as you type. ImGui uses whatever Buf/BufSize we hand back
// and cuts the text to fit, so reporting less than it asked for (at the size limit, or if realloc fails) is safe
static int shader_source_resize_callback(ImGuiInputTextCallbackData* data) {
	if (data->EventFlag != ImGuiInputTextFlags_CallbackResize) { return 0; }

	AquaShaderSource* source = (AquaShaderSource*) data->UserData;

	usize wanted = (usize) data->BufSize;
	if (wanted > source->capacity) {
		usize new_capacity = source->capacity * 2;
		if (new_capacity < wanted) { new_capacity = wanted; }
		if (new_capacity > AQUA_SHADER_SOURCE_MAX_SIZE + 1) { new_capacity = AQUA_SHADER_SOURCE_MAX_SIZE + 1; }

		if (new_capacity > source->capacity) {
			char* text = (char*) realloc(source->text, new_capacity);
			if (text) {
				source->text = text;
				source->capacity = new_capacity;
			}
		}
	}

	data->Buf = source->text;
	data->BufSize = (int) source->capacity;
	return 0;
}

static void shader_source_editor(const char* label, AquaShaderSource* source) {
	igText("%s (%zu / %d bytes)", label, strlen(source->text), AQUA_SHADER_SOURCE_MAX_SIZE);
	igPushID_Str(label);
	igInputTextMultiline("##source", source->text, source->capacity, (ImVec2) { -1.0f, 0.0f }, ImGuiInputTextFlags_CallbackResize, shader_source_resize_callback, source);
	igPopID();
}

void aqua_shader_imgui_collapsible_header(AquaShader* shader, const char* header_label) {
	if (igCollapsingHeader_BoolPtr(header_label, NULL, ImGuiTreeNodeFlags_None)) {
		igPushID_Ptr(shader);

		igText("Program ID: %u", shader->program);
		igText("Projection Uniform Location: %d", shader->projection_uniform_location);
		igText("View Uniform Location: %d", shader->view_uniform_location);
		igText("Model Uniform Location: %d", shader->model_uniform_location);
		igText("Color Uniform Location: %d", shader->color_uniform_location);
		igText("Color Strength Uniform Location: %d", shader->color_strength_uniform_location);
		igSeparator();

		shader_source_editor("Vertex Shader Source", &shader->vertex_source);
		shader_source_editor("Fragment Shader Source", &shader->fragment_source);
		igSeparator();

		if (igButton("Reload", (ImVec2) { 0.0f, 0.0f })) {
			shader->last_reload_failed = !aqua_shader_reload(shader);
		}
		if (shader->last_reload_failed) {
			igSameLine(0.0f, -1.0f);
			igTextColored((ImVec4) { 1.0f, 0.4f, 0.4f, 1.0f }, "Reload failed, still using the previous program (see console)");
		}
		igSeparator();

		igPopID();
	}
}

void aqua_shader_destroy(AquaShader* shader) {
	if (shader->program != 0) { glDeleteProgram(shader->program); }
	shader_source_free(&shader->vertex_source);
	shader_source_free(&shader->fragment_source);
	shader->program = 0;
}
