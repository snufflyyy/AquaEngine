#include "shader.h"

#include <stdbool.h>
#include <stdio.h>

#include <glad/glad.h>
#include <string.h>

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include <cimgui.h>
#include <cimgui_impl.h>

#include "utils/file/file.h"

static bool shader_create_module(GLuint* result, const char* shader_source, GLenum shader_type);

AquaShader aqua_shader_create(const char* vertex_shader_source_path, const char* fragment_shader_source_path) {
    const char* vertex_shader_source = aqua_file_to_string(vertex_shader_source_path);
    if (!vertex_shader_source) {
    	fprintf(stderr, "[ERROR] [Aqua] [Shader] Failed to open vertex shader source file: %s!\n", vertex_shader_source_path);
    	return (AquaShader) { .failed = true };
    }

    if (strnlen(vertex_shader_source, AQUA_SHADER_SOURCE_BUFFER_SIZE) >= AQUA_SHADER_SOURCE_BUFFER_SIZE) {
	   	fprintf(stderr, "[ERROR] [Aqua] [Shader] Vertex shader source length is too large!\n");
		aqua_file_free_string((char*) vertex_shader_source);
	   	return (AquaShader) { .failed = true };
    }

    const char* fragment_shader_source = aqua_file_to_string(fragment_shader_source_path);
    if (!fragment_shader_source) {
    	fprintf(stderr, "[ERROR] [Aqua] [Shader] Failed to open fragment shader source file: %s!\n", fragment_shader_source_path);
    	aqua_file_free_string((char*) vertex_shader_source);
     	return (AquaShader) { .failed = true };
    }

    if (strnlen(fragment_shader_source, AQUA_SHADER_SOURCE_BUFFER_SIZE) >= AQUA_SHADER_SOURCE_BUFFER_SIZE) {
	   	fprintf(stderr, "[ERROR] [Aqua] [Shader] Fragment shader source length is too large!\n");
		aqua_file_free_string((char*) vertex_shader_source);
		aqua_file_free_string((char*) fragment_shader_source);
		return (AquaShader) { .failed = true };
    }

    GLuint vertex_shader;
    if (!shader_create_module(&vertex_shader, vertex_shader_source, GL_VERTEX_SHADER)) {
    	fprintf(stderr, "[ERROR] [Aqua] [Shader] Failed to create vertex shader module!\n");
     	return (AquaShader) { .failed = true };
    }
    GLuint fragment_shader;
    if (!shader_create_module(&fragment_shader, fragment_shader_source, GL_FRAGMENT_SHADER)) {
   		fprintf(stderr, "[ERROR] [Aqua] [Shader] Failed to create fragment shader module!\n");
    	glDeleteShader(vertex_shader);
    	return (AquaShader) { .failed = true };
    }

    AquaShader shader;

    shader.program = glCreateProgram();
    if (shader.program == 0) {
   		fprintf(stderr, "[ERROR] [Aqua] [Shader] Failed to create OpenGL shader program!\n");
    	glDeleteShader(vertex_shader);
    	glDeleteShader(fragment_shader);
    	return (AquaShader) { .failed = true };
    }
    glAttachShader(shader.program, vertex_shader);
    glAttachShader(shader.program, fragment_shader);
    glLinkProgram(shader.program);

    GLint shader_program_link_status;
    glGetProgramiv(shader.program, GL_LINK_STATUS, &shader_program_link_status);
    if (shader_program_link_status == GL_FALSE) {
        char info_log[512];
        glGetProgramInfoLog(shader.program, sizeof(info_log), NULL, info_log);
        fprintf(stderr, "[ERROR] [Aqua] [Shader] Failed to link shader program:\n%s\n", info_log);

        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);
        glDeleteProgram(shader.program);
       	return (AquaShader) { .failed = true };
    }

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    strncpy(shader.vertex_shader_source, vertex_shader_source, AQUA_SHADER_SOURCE_BUFFER_SIZE);
    strncpy(shader.fragment_shader_source, fragment_shader_source, AQUA_SHADER_SOURCE_BUFFER_SIZE);

    aqua_file_free_string((char*) vertex_shader_source);
    aqua_file_free_string((char*) fragment_shader_source);

    shader.projection_uniform_location = aqua_shader_get_uniform_location(shader, "projection");
    shader.view_uniform_location = aqua_shader_get_uniform_location(shader, "view");
    shader.model_uniform_location = aqua_shader_get_uniform_location(shader, "model");

    shader.failed = false;

    return shader;
}

static bool shader_create_module(GLuint* result, const char* shader_source, GLenum shader_type) {
    GLuint shader_module = glCreateShader(shader_type);
    if (shader_module == 0) {
   		fprintf(stderr, "[ERROR] [Aqua] [Shader] Failed to create OpenGL shader module!\n");
     	return false;
    }
    glShaderSource(shader_module, 1, (const char**) &shader_source, NULL);
    glCompileShader(shader_module);

    GLint shader_compile_result;
    glGetShaderiv(shader_module, GL_COMPILE_STATUS, &shader_compile_result);
    if (shader_compile_result == GL_FALSE) {
        char info_log[512];
        glGetShaderInfoLog(shader_module, sizeof(info_log), NULL, info_log);

        const char* shader_type_string;
        switch (shader_type) {
            case GL_VERTEX_SHADER: shader_type_string = "vertex"; break;
            case GL_FRAGMENT_SHADER: shader_type_string = "fragment"; break;
            default: shader_type_string = "unknown"; break;
        }

        fprintf(stderr, "[ERROR] [Aqua] [Shader] Failed to create %s shader module: \n%s\n", shader_type_string, info_log);

        glDeleteShader(shader_module);
        return false;
    }

    *result = shader_module;
    return true;
}

void aqua_shader_reload(AquaShader* shader) {
	if (shader->failed) { return; }

	glDeleteProgram(shader->program);

 	GLuint vertex_shader;
    if (!shader_create_module(&vertex_shader, shader->vertex_shader_source, GL_VERTEX_SHADER)) {
    	fprintf(stderr, "[ERROR] [Aqua] [Shader] Failed to create vertex shader module!\n");
     	return;
    }
    GLuint fragment_shader;
    if (!shader_create_module(&fragment_shader, shader->fragment_shader_source, GL_FRAGMENT_SHADER)) {
   		fprintf(stderr, "[ERROR] [Aqua] [Shader] Failed to create fragment shader module!\n");
    	glDeleteShader(vertex_shader);
     	return;
    }

    shader->program = glCreateProgram();
    if (shader->program == 0) {
   		fprintf(stderr, "[ERROR] [Aqua] [Shader] Failed to create OpenGL shader program!\n");
    	glDeleteShader(vertex_shader);
    	glDeleteShader(fragment_shader);
    	return;
    }
    glAttachShader(shader->program, vertex_shader);
    glAttachShader(shader->program, fragment_shader);
    glLinkProgram(shader->program);

    GLint shader_program_link_status;
    glGetProgramiv(shader->program, GL_LINK_STATUS, &shader_program_link_status);
    if (shader_program_link_status == GL_FALSE) {
        char info_log[512];
        glGetProgramInfoLog(shader->program, sizeof(info_log), NULL, info_log);
        fprintf(stderr, "[ERROR] [Aqua] [Shader] Failed to link shader program:\n%s\n", info_log);

        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);
        glDeleteProgram(shader->program);
        return;
    }

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    shader->projection_uniform_location = aqua_shader_get_uniform_location(*shader, "projection");
    shader->view_uniform_location = aqua_shader_get_uniform_location(*shader, "view");
    shader->model_uniform_location = aqua_shader_get_uniform_location(*shader, "model");
}

AquaShaderUniformLocation aqua_shader_get_uniform_location(AquaShader shader, const char* uniform_name) {
	return glGetUniformLocation(shader.program, uniform_name);
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

void aqua_shader_imgui_collapsible_header(AquaShader* shader, const char* header_label) {
	if (igCollapsingHeader_BoolPtr(header_label, NULL, ImGuiTreeNodeFlags_None)) {
		igText("Program ID: %u", shader->program);
		igText("Projection Uniform Location: %d", shader->projection_uniform_location);
		igText("View Uniform Location: %d", shader->view_uniform_location);
		igText("Model Uniform Location: %d", shader->model_uniform_location);
		igSeparator();
		igInputTextMultiline("Vertex Shader Source", shader->vertex_shader_source, AQUA_SHADER_SOURCE_BUFFER_SIZE, (ImVec2) { 0.0f, 0.0f }, ImGuiInputTextFlags_None, 0, NULL);
		igInputTextMultiline("Fragment Shader Source", shader->fragment_shader_source, AQUA_SHADER_SOURCE_BUFFER_SIZE, (ImVec2) { 0.0f, 0.0f }, ImGuiInputTextFlags_None, 0, NULL);
		igSeparator();
		igText("NOTE: If you are reloading the currently bound shader,\nyou need to rebind it in the renderer properties window.");
		if (igButton("Reload", (ImVec2) { 0.0f, 0.0f })) {
			aqua_shader_reload(shader);
		}
		igSeparator();
	}
}

void aqua_shader_destroy(AquaShader shader) {
	if (shader.program == 0) { return; }

    glDeleteProgram(shader.program);
}
