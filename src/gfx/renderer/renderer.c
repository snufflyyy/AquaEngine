#include "gfx/renderer/renderer.h"

#include "event/event.h"
#include "gfx/material/material.h"
#include "gfx/mesh/mesh.h"
#include "gfx/renderer/resource-manager.h"
#include "gfx/shader/shader.h"
#include "gfx/texture/texture.h"
#include "gfx/window/window.h"
#include <aqua/types.h>

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include <SDL3/SDL.h>
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include <cimgui.h>
#include <cimgui_impl.h>

static bool renderer_can_draw(const AquaRenderer* renderer) {
	if (aqua_renderer_resource_manager_is_shader_valid(&renderer->resource_manager, renderer->current_shader) && renderer->current_camera) { return true; }

	fprintf(stderr, "[ERROR] [Aqua] [Renderer] Draw called without a bound material and camera, skipping!\n");
	return false;
}

AquaRenderer* aqua_renderer_create(AquaRendererProperties properties, AquaWindow* window) {
	bool resource_manager_initialized = false;
	bool material_manager_initialized = false;
	bool model_manager_initialized = false;
	bool imgui_sdl_initialized = false;
	bool imgui_gl_initialized = false;

	AquaRenderer* renderer = (AquaRenderer*) calloc(1, sizeof(AquaRenderer));
	if (!renderer) {
		fprintf(stderr, "[ERROR] [Aqua] [Renderer] Failed to allocate memory for renderer!\n");
		goto error;
	}

	renderer->gl_context = SDL_GL_CreateContext(window->sdl_window);
	if (!renderer->gl_context) {
		fprintf(stderr, "[ERROR] [Aqua] [Renderer] Failed to create SDL OpenGL context!\n");
		goto error;
	}

	if (!SDL_GL_MakeCurrent(window->sdl_window, renderer->gl_context)) {
		fprintf(stderr, "[ERROR] [Aqua] [Renderer] Failed to set SDL OpenGL context as current!\n");
		goto error;
	}

	if (!gladLoadGLLoader((GLADloadproc) SDL_GL_GetProcAddress)) {
		fprintf(stderr, "[ERROR] [Aqua] [Renderer] Failed load GLAD!\n");
		goto error;
	}

    glEnable(GL_MULTISAMPLE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    // the viewport is in pixels, which differs from the window's logical size on high-DPI displays
    int pixel_width = (int) window->properties.width;
    int pixel_height = (int) window->properties.height;
    if (!SDL_GetWindowSizeInPixels(window->sdl_window, &pixel_width, &pixel_height)) {
    	fprintf(stderr, "[WARNING] [Aqua] [Renderer] Failed to get window size in pixels, using the logical size: %s\n", SDL_GetError());
    }
    glViewport(0, 0, pixel_width, pixel_height);

    aqua_renderer_resource_manager_init(&renderer->resource_manager);
    resource_manager_initialized = true;
    renderer->current_shader = AQUA_SHADER_HANDLE_INVALID;

    renderer->base_material_shader = aqua_renderer_create_shader(renderer, AQUA_ENGINE_SHADER_DIRECTORY "/material/material.vert", AQUA_ENGINE_SHADER_DIRECTORY "/material/material.frag");
    if (renderer->base_material_shader == AQUA_SHADER_HANDLE_INVALID) {
    	fprintf(stderr, "[ERROR] [Aqua] [Renderer] Failed to create base material shader from '" AQUA_ENGINE_SHADER_DIRECTORY "'!\n");
    	fprintf(stderr, "[ERROR] [Aqua] [Renderer] The working directory must be the project root (where '" AQUA_ENGINE_SHADER_DIRECTORY "' is), not the build directory.\n");
    	goto error;
    }

    const u8 white_pixel[4] = { 255, 255, 255, 255 };
    renderer->default_texture = aqua_renderer_create_texture_from_pixels(renderer, white_pixel, 1, 1);
    if (renderer->default_texture == AQUA_TEXTURE_HANDLE_INVALID) {
    	fprintf(stderr, "[ERROR] [Aqua] [Renderer] Failed to create default texture!\n");
    	goto error;
    }

    aqua_material_manager_init(&renderer->material_manager);
    material_manager_initialized = true;
    renderer->default_material = aqua_material_manager_create(&renderer->material_manager, &renderer->resource_manager, renderer->base_material_shader, renderer->default_texture, (AquaColor) { 1.0f, 1.0f, 1.0f, 1.0f });
    if (renderer->default_material == AQUA_MATERIAL_HANDLE_INVALID) {
    	fprintf(stderr, "[ERROR] [Aqua] [Renderer] Failed to create default material!\n");
    	goto error;
    }

    aqua_model_manager_init(&renderer->model_manager);
    model_manager_initialized = true;

    renderer->imgui_context = igCreateContext(NULL);
    if (!renderer->imgui_context) { goto error; }
    renderer->started_imgui_frame = false;
    renderer->imgui_process_input = true;

    ImGuiIO* imgui_io = igGetIO_ContextPtr(renderer->imgui_context);
    imgui_io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    #ifdef IMGUI_HAS_DOCK
    	imgui_io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    #endif

    imgui_sdl_initialized = ImGui_ImplSDL3_InitForOpenGL(window->sdl_window, renderer->gl_context);
    if (!imgui_sdl_initialized) { goto error; }
    imgui_gl_initialized = ImGui_ImplOpenGL3_Init(NULL);
    if (!imgui_gl_initialized) { goto error; }

    renderer->info = (AquaRendererInfo) {
        .gl_video_driver = SDL_GetCurrentVideoDriver(),
        .gl_vendor = (const char*) glGetString(GL_VENDOR),
        .gl_renderer = (const char*) glGetString(GL_RENDERER),
        .gl_opengl_version = (const char*) glGetString(GL_VERSION),
        .gl_glsl_version = (const char*) glGetString(GL_SHADING_LANGUAGE_VERSION),
    };
    if (!aqua_renderer_apply_properties(renderer, properties, true)) { goto error; }

   	#ifndef NDEBUG
      	printf("[INFO] [Aqua] [Renderer] Created Renderer\n");
    #endif

	return renderer;

error:
	if (renderer) {
		if (imgui_gl_initialized) { ImGui_ImplOpenGL3_Shutdown(); }
		if (imgui_sdl_initialized) { ImGui_ImplSDL3_Shutdown(); }
		if (renderer->imgui_context) { igDestroyContext(renderer->imgui_context); }
		if (model_manager_initialized) { aqua_model_manager_destroy(&renderer->model_manager, renderer); }
		if (material_manager_initialized) { aqua_material_manager_destroy(&renderer->material_manager); }
		if (resource_manager_initialized) { aqua_renderer_resource_manager_destroy(&renderer->resource_manager); }
		if (renderer->gl_context) { SDL_GL_DestroyContext(renderer->gl_context); }
		free(renderer);
	}
	return NULL;
}

void aqua_renderer_event(AquaRenderer* renderer, const AquaEvent* event) {
	(void) renderer; // The viewport belongs to the current graphics context.
	switch (event->type) {
		case AQUA_EVENT_TYPE_WINDOW_PIXEL_SIZE_CHANGED: glViewport(0, 0, (GLsizei) event->window.new_width, (GLsizei) event->window.new_height); break;
		default: break;
	}
}

void aqua_renderer_imgui_native_event(AquaRenderer* renderer, const SDL_Event* event) {
	if (renderer->imgui_process_input) { ImGui_ImplSDL3_ProcessEvent(event); }
}

void aqua_renderer_update(AquaRenderer* renderer, f64 time) {
	if (renderer->properties.rainbow_clear_color) {
		float t = (float) time;
		float phase = renderer->properties.rainbow_clear_color_speed * t;

		AquaColor new_clear_color = {
			0.5f + 0.5f * sinf(phase + 0.0f),
			0.5f + 0.5f * sinf(phase + 2.0943951f), // 2pi/3
			0.5f + 0.5f * sinf(phase + 4.1887902f) // 4pi/3
		};
		glClearColor(new_clear_color[0], new_clear_color[1], new_clear_color[2], renderer->properties.clear_color[3]);
	}
}

void aqua_renderer_clear(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

static void renderer_bind_shader(AquaRenderer* renderer, AquaShaderHandle shader) {
	if (!aqua_renderer_resource_manager_is_shader_valid(&renderer->resource_manager, shader)) { return; }

	glUseProgram(renderer->resource_manager.shaders[shader].program);
}

static void renderer_bind_texture(AquaRenderer* renderer, AquaTextureHandle texture, u32 texture_unit) {
	// invalid or destroyed textures draw white
	if (!aqua_renderer_resource_manager_is_texture_valid(&renderer->resource_manager, texture)) { texture = renderer->default_texture; }

	glActiveTexture(GL_TEXTURE0 + texture_unit);
	glBindTexture(GL_TEXTURE_2D, renderer->resource_manager.textures[texture].id);
}

void aqua_renderer_bind_material(AquaRenderer* renderer, AquaMaterialHandle material_handle) {
    AquaMaterial* material = aqua_material_manager_get(&renderer->material_manager, material_handle);
    if (!material) {
    	if (material_handle != AQUA_MATERIAL_HANDLE_INVALID) {
    		// a handle that was valid once: its material was destroyed but it's still being drawn
            fprintf(stderr, "[WARNING] [Aqua] [Renderer] Bound destroyed material %zu, using the default material!\n", material_handle);
    	}
    	material = aqua_material_manager_get(&renderer->material_manager, renderer->default_material);
    }

    if (!aqua_renderer_resource_manager_is_shader_valid(&renderer->resource_manager, material->shader)) {
    	renderer->current_shader = AQUA_SHADER_HANDLE_INVALID; // draws get skipped
    	return;
    }
    AquaShader* shader = &renderer->resource_manager.shaders[material->shader];

    renderer_bind_shader(renderer, material->shader);
    renderer_bind_texture(renderer, material->texture, 0);

    aqua_shader_set_vec4_uniform(shader->color_uniform_location, material->color);
    aqua_shader_set_float_uniform(shader->color_strength_uniform_location, material->color_strength);

    renderer->current_shader = material->shader;
}

AquaMaterialHandle aqua_renderer_create_material(AquaRenderer* renderer, AquaTextureHandle texture, AquaColor color) {
	return aqua_material_manager_create(&renderer->material_manager, &renderer->resource_manager, renderer->base_material_shader, texture, color);
}

void aqua_renderer_destroy_material(AquaRenderer* renderer, AquaMaterialHandle material) {
	if (material == renderer->default_material) {
		fprintf(stderr, "[ERROR] [Aqua] [Renderer] The default material can't be destroyed!\n");
		return;
	}
	aqua_material_manager_destroy_material(&renderer->material_manager, material);
}

void aqua_renderer_set_material_shader(AquaRenderer* renderer, AquaMaterialHandle material, AquaShaderHandle shader) {
	AquaMaterialProperties properties = aqua_renderer_get_material_properties(renderer, material);
	properties.shader = shader;
	(void) aqua_renderer_set_material_properties(renderer, material, properties);
}

AquaModelHandle aqua_renderer_create_model(AquaRenderer* renderer, const char* model_path) {
	return aqua_model_manager_create(&renderer->model_manager, renderer, model_path);
}

void aqua_renderer_destroy_model(AquaRenderer* renderer, AquaModelHandle model) {
	aqua_model_manager_destroy_model(&renderer->model_manager, renderer, model);
}

void aqua_renderer_bind_camera(AquaRenderer* renderer, AquaCamera* camera) {
    renderer->current_camera = camera;
}

AquaShaderHandle aqua_renderer_create_shader(AquaRenderer* renderer, const char* vertex_shader_source_path, const char* fragment_shader_source_path) {
	return aqua_renderer_resource_manager_create_shader(&renderer->resource_manager, vertex_shader_source_path, fragment_shader_source_path);
}

AquaTextureHandle aqua_renderer_create_texture(AquaRenderer* renderer, const char* image_path) {
	return aqua_renderer_resource_manager_create_texture(&renderer->resource_manager, image_path);
}

AquaTextureHandle aqua_renderer_create_texture_from_memory(AquaRenderer* renderer, const u8* buffer, u32 buffer_length) {
    return aqua_renderer_resource_manager_create_texture_from_memory(&renderer->resource_manager, buffer, buffer_length);
}

AquaTextureHandle aqua_renderer_create_texture_from_pixels(AquaRenderer* renderer, const u8* rgba_pixels, u32 width, u32 height) {
    return aqua_renderer_resource_manager_create_texture_from_pixels(&renderer->resource_manager, rgba_pixels, width, height);
}

AquaMeshHandle aqua_renderer_create_mesh(AquaRenderer* renderer, const AquaVertex* vertices, u32 vertices_count, const u32* indices, u32 indices_count) {
	assert(sizeof(u32) == sizeof(GLuint) && "OpenGL indices must be 32-bit");
	return aqua_renderer_resource_manager_create_mesh(&renderer->resource_manager, vertices, vertices_count, indices, indices_count);
}

void aqua_renderer_destroy_shader(AquaRenderer* renderer, AquaShaderHandle shader) {
	if (shader == renderer->base_material_shader) {
		fprintf(stderr, "[ERROR] [Aqua] [Renderer] The base material shader can't be destroyed!\n");
		return;
	}

	if (shader == renderer->current_shader) { renderer->current_shader = AQUA_SHADER_HANDLE_INVALID; }
	aqua_renderer_resource_manager_destroy_shader(&renderer->resource_manager, shader);
}

void aqua_renderer_destroy_texture(AquaRenderer* renderer, AquaTextureHandle texture) {
	if (texture == renderer->default_texture) {
		fprintf(stderr, "[ERROR] [Aqua] [Renderer] The default texture can't be destroyed!\n");
		return;
	}

	aqua_renderer_resource_manager_destroy_texture(&renderer->resource_manager, texture);
}

void aqua_renderer_destroy_mesh(AquaRenderer* renderer, AquaMeshHandle mesh) {
	aqua_renderer_resource_manager_destroy_mesh(&renderer->resource_manager, mesh);
}

void aqua_renderer_draw_mesh(AquaRenderer* renderer, AquaMeshHandle mesh, mat4 transform) {
    if (!renderer_can_draw(renderer)) { return; }
    if (!aqua_renderer_resource_manager_is_mesh_valid(&renderer->resource_manager, mesh)) { return; }

    AquaMesh* m = &renderer->resource_manager.meshes[mesh];

    AquaShader* shader = &renderer->resource_manager.shaders[renderer->current_shader];
    aqua_shader_set_mat4_uniform(shader->view_uniform_location, renderer->current_camera->view);
    aqua_shader_set_mat4_uniform(shader->projection_uniform_location, renderer->current_camera->projection);
    aqua_shader_set_mat4_uniform(shader->model_uniform_location, transform);

    glBindVertexArray(m->vao);
    glDrawElements(GL_TRIANGLES, (GLsizei) m->indices_count, GL_UNSIGNED_INT, 0);
}

void aqua_renderer_draw_submesh(AquaRenderer* renderer, AquaMeshHandle mesh, u32 indices_start, u32 indices_count, mat4 transform) {
    if (!renderer_can_draw(renderer)) { return; }
    if (!aqua_renderer_resource_manager_is_mesh_valid(&renderer->resource_manager, mesh)) { return; }

    AquaMesh* m = &renderer->resource_manager.meshes[mesh];

    // written as a subtraction so indices_start + indices_count can't overflow
    if (indices_start > m->indices_count || indices_count > m->indices_count - indices_start) {
        fprintf(stderr, "[ERROR] [Aqua] [Renderer] Submesh range [%u, %u + %u) is outside mesh %zu (%u indices), skipping!\n", indices_start, indices_start, indices_count, mesh, m->indices_count);
    	return;
    }

    AquaShader* shader = &renderer->resource_manager.shaders[renderer->current_shader];
    aqua_shader_set_mat4_uniform(shader->view_uniform_location, renderer->current_camera->view);
    aqua_shader_set_mat4_uniform(shader->projection_uniform_location, renderer->current_camera->projection);
    aqua_shader_set_mat4_uniform(shader->model_uniform_location, transform);

    glBindVertexArray(m->vao);
    glDrawElements(GL_TRIANGLES, (GLsizei) indices_count, GL_UNSIGNED_INT, (void*)(uintptr_t)(indices_start * sizeof(GLuint)));
}

void aqua_renderer_draw_static_model(AquaRenderer* renderer, AquaModelHandle model_handle, mat4 transform) {
	const AquaStaticModel* model = aqua_model_manager_get(&renderer->model_manager, model_handle);
	if (!model || model->failed) { return; }

	for (u32 i = 0; i < model->submeshes_count; i++) {
		const AquaStaticModelSubMesh* submesh = &model->submeshes[i];
		aqua_renderer_bind_material(renderer, submesh->material);
		aqua_renderer_draw_submesh(renderer, submesh->mesh, submesh->indices_start, submesh->indices_count, transform);
	}
}

void aqua_renderer_draw_model_instance(AquaRenderer* renderer, const AquaModelInstance* instance) {
	if (!instance) { return; }

	// cglm's copy takes a mutable source parameter but only reads it.
	mat4 transform;
	glm_mat4_copy((vec4*) instance->transform, transform);
	aqua_renderer_draw_static_model(renderer, instance->model, transform);
}

void aqua_renderer_imgui_begin(AquaRenderer* renderer) {
 	ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    igNewFrame();

    renderer->started_imgui_frame = true;
}

void aqua_renderer_imgui_update(AquaRenderer* renderer) {
	aqua_renderer_resource_manager_imgui_update(&renderer->resource_manager);
	aqua_material_manager_imgui_update(&renderer->material_manager, &renderer->resource_manager);
	aqua_model_manager_imgui_update(&renderer->model_manager);

	if (renderer->properties.show_properties_window) {
		aqua_renderer_imgui_properties_window(renderer);
	}
}

void aqua_renderer_imgui_end(AquaRenderer* renderer) {
	if (!renderer->started_imgui_frame) { return; }

    igEndFrame();
    renderer->started_imgui_frame = false;
}

void aqua_renderer_imgui_draw(AquaRenderer* renderer) {
	if (renderer->started_imgui_frame) { return; }

    igRender();
    ImGui_ImplOpenGL3_RenderDrawData(igGetDrawData());
}

void aqua_renderer_destroy(AquaRenderer* renderer) {
	if (!renderer) { return; }

 	ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
	igDestroyContext(renderer->imgui_context);
	aqua_model_manager_destroy(&renderer->model_manager, renderer);
	aqua_material_manager_destroy(&renderer->material_manager);
	aqua_renderer_resource_manager_destroy(&renderer->resource_manager);
	SDL_GL_DestroyContext(renderer->gl_context);
	free(renderer);

 	#ifndef NDEBUG
    	printf("[INFO] [Aqua] [Renderer] Renderer Destroyed\n");
    #endif
}
