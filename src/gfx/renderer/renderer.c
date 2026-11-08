#include "gfx/renderer/renderer.h"

#include "event/event.h"
#include "gfx/material/material.h"
#include "gfx/mesh/mesh.h"
#include "gfx/renderer/resource-manager.h"
#include "gfx/shader/shader.h"
#include "gfx/texture/texture.h"
#include "gfx/window/window.h"
#include "utils/base-types.h"

#include <stdlib.h>
#include <stdio.h>

#include <SDL3/SDL.h>
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include <cimgui.h>
#include <cimgui_impl.h>

static bool renderer_can_draw(AquaRenderer* renderer) {
	if (aqua_renderer_resource_manager_is_shader_valid(&renderer->resource_manager, renderer->current_shader) && renderer->current_camera) { return true; }

	static bool warned = false;
	if (!warned) {
		fprintf(stderr, "[ERROR] [Aqua] [Renderer] Draw called without a bound material and camera, skipping!\n");
		warned = true;
	}
	return false;
}

AquaRenderer* aqua_renderer_create(AquaRendererCreateProperties create_properties, AquaWindow* window) {
	AquaRenderer* renderer = (AquaRenderer*) calloc(1, sizeof(AquaRenderer));
	if (!renderer) {
		fprintf(stderr, "[ERROR] [Aqua] [Renderer] Failed to allocate memory for renderer!\n");
		return NULL;
	}

	renderer->gl_context = SDL_GL_CreateContext(window->sdl_window);
    if (!renderer->gl_context) {
   		fprintf(stderr, "[ERROR] [Aqua] [Renderer] Failed to create SDL OpenGL context!\n");
       	free(renderer);
       	return NULL;
    }

    if (!SDL_GL_MakeCurrent(window->sdl_window, renderer->gl_context)) {
  		fprintf(stderr, "[ERROR] [Aqua] [Renderer] Failed to set SDL OpenGL context as current!\n");
   		SDL_GL_DestroyContext(renderer->gl_context);
      	free(renderer);
      	return NULL;
    }

    if (!gladLoadGLLoader((GLADloadproc) SDL_GL_GetProcAddress)) {
  		fprintf(stderr, "[ERROR] [Aqua] [Renderer] Failed load GLAD!\n");
   		SDL_GL_DestroyContext(renderer->gl_context);
       	free(renderer);
       	return NULL;
    }

    glEnable(GL_MULTISAMPLE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    glViewport(0, 0, (int) window->properties.width, (int) window->properties.height);

    aqua_renderer_resource_manager_init(&renderer->resource_manager);
    renderer->current_shader = AQUA_SHADER_HANDLE_INVALID;

    renderer->base_material_shader = aqua_renderer_create_shader(renderer, AQUA_ENGINE_SHADER_DIRECTORY "/material/material.vert", AQUA_ENGINE_SHADER_DIRECTORY "/material/material.frag");
    if (renderer->base_material_shader == AQUA_SHADER_HANDLE_INVALID) {
    	fprintf(stderr, "[ERROR] [Aqua] [Renderer] Failed to create base material shader from '" AQUA_ENGINE_SHADER_DIRECTORY "'!\n");
    	fprintf(stderr, "[ERROR] [Aqua] [Renderer] The working directory must be the project root (where '" AQUA_ENGINE_SHADER_DIRECTORY "' is), not the build directory.\n");
    	aqua_renderer_resource_manager_destroy(&renderer->resource_manager);
     	SDL_GL_DestroyContext(renderer->gl_context);
       	free(renderer);
       	return NULL;
    }

    const u8 white_pixel[4] = { 255, 255, 255, 255 };
    renderer->default_texture = aqua_renderer_create_texture_from_pixels(renderer, white_pixel, 1, 1);
    if (renderer->default_texture == AQUA_TEXTURE_HANDLE_INVALID) {
    	fprintf(stderr, "[ERROR] [Aqua] [Renderer] Failed to create default texture!\n");
    	aqua_renderer_resource_manager_destroy(&renderer->resource_manager);
     	SDL_GL_DestroyContext(renderer->gl_context);
       	free(renderer);
       	return NULL;
    }

    aqua_material_manager_init(&renderer->material_manager);
    renderer->default_material = aqua_material_manager_create(&renderer->material_manager, renderer->base_material_shader, renderer->default_texture, (AquaColor) { 1.0f, 1.0f, 1.0f, 1.0f });
    if (renderer->default_material == AQUA_MATERIAL_HANDLE_INVALID) {
    	fprintf(stderr, "[ERROR] [Aqua] [Renderer] Failed to create default material!\n");
    	aqua_renderer_resource_manager_destroy(&renderer->resource_manager);
     	SDL_GL_DestroyContext(renderer->gl_context);
       	free(renderer);
       	return NULL;
    }

    renderer->last_performance_counter = SDL_GetPerformanceCounter();
    renderer->performance_frequency = (double) SDL_GetPerformanceFrequency();

    renderer->fps = 0.0f;
    renderer->delta_time = 0.0;

    renderer->imgui_context = igCreateContext(NULL);
    renderer->started_imgui_frame = false;

    ImGuiIO* imgui_io = igGetIO_ContextPtr(renderer->imgui_context);
    imgui_io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    #ifdef IMGUI_HAS_DOCK
      imgui_io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    #endif

    ImGui_ImplSDL3_InitForOpenGL(window->sdl_window, renderer->gl_context);
    ImGui_ImplOpenGL3_Init(NULL);

    renderer->properties = (AquaRendererProperties) {
    	.vsync = create_properties.vsync,
    	.rainbow_clear_color = create_properties.rainbow_clear_color,
     	.rainbow_clear_color_speed = create_properties.rainbow_clear_color_speed,
      	.gl_video_driver = (char*) SDL_GetCurrentVideoDriver(),
      	.gl_vender = (char*) glGetString(GL_VENDOR),
      	.gl_renderer = (char*) glGetString(GL_RENDERER),
      	.gl_opengl_version = (char*) glGetString(GL_VERSION),
       	.gl_glsl_version = (char*) glGetString(GL_SHADING_LANGUAGE_VERSION),
        .show_performance_window = create_properties.show_performance_window,
        .imgui_process_input = true,
        .show_properties_window = create_properties.show_properties_window,
    };
    glm_vec3_copy(create_properties.clear_color, renderer->properties.clear_color);

    aqua_renderer_set_vsync(renderer, renderer->properties.vsync);

   	#ifndef NDEBUG
      	printf("[INFO] [Aqua] [Renderer] Created Renderer\n");
    #endif

	return renderer;
}

void aqua_renderer_event(AquaRenderer *renderer, AquaEvent *event) {
	switch (event->type) {
		case AQUA_EVENT_TYPE_WINDOW_RESIZED: glViewport(0, 0, (GLsizei) event->window.new_width, (GLsizei) event->window.new_height); break;
		default: break;
	}
	if (renderer->properties.imgui_process_input) {
		ImGui_ImplSDL3_ProcessEvent(&event->sdl_event);
	}
}

void aqua_renderer_update(AquaRenderer* renderer) {
	u64 performance_counter = SDL_GetPerformanceCounter();
	renderer->delta_time = (double) (performance_counter - renderer->last_performance_counter) / renderer->performance_frequency;
	renderer->last_performance_counter = performance_counter;

	if (renderer->delta_time > 0.0) {
	    renderer->fps = 1.0f / ((float) renderer->delta_time);
	}

	if (renderer->properties.rainbow_clear_color) {
		float t = (float) ((double) performance_counter / (double) renderer->performance_frequency);
		float phase = renderer->properties.rainbow_clear_color_speed * t;

		AquaColor new_clear_color = {
			0.5f + 0.5f * sinf(phase + 0.0f),
			0.5f + 0.5f * sinf(phase + 2.0943951f), // 2pi/3
			0.5f + 0.5f * sinf(phase + 4.1887902f) // 4pi/3
		};
		aqua_renderer_set_clear_color(new_clear_color);
	}
}

void aqua_renderer_set_vsync(AquaRenderer* renderer, bool value) {
	SDL_GL_SetSwapInterval((value) ? 1 : 0);
	renderer->properties.vsync = value;
}

void aqua_renderer_set_clear_color(AquaColor color) {
    glClearColor(color[0], color[1], color[2], 1.0f);
}

void aqua_renderer_clear(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void aqua_renderer_bind_shader(AquaRenderer* renderer, AquaShaderHandle shader) {
	if (!aqua_renderer_resource_manager_is_shader_valid(&renderer->resource_manager, shader)) { return; }

	glUseProgram(renderer->resource_manager.shaders[shader].program);
}

void aqua_renderer_rebind_shader(AquaRenderer* renderer) {
	if (!aqua_renderer_resource_manager_is_shader_valid(&renderer->resource_manager, renderer->current_shader)) { return; }
	glUseProgram(renderer->resource_manager.shaders[renderer->current_shader].program);
}

void aqua_renderer_bind_texture(AquaRenderer* renderer, AquaTextureHandle texture) {
	// invalid or destroyed textures draw white
	if (!aqua_renderer_resource_manager_is_texture_valid(&renderer->resource_manager, texture)) { texture = renderer->default_texture; }

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, renderer->resource_manager.textures[texture].id);
}

void aqua_renderer_bind_material(AquaRenderer* renderer, AquaMaterialHandle material_handle) {
    AquaMaterial* material = aqua_material_manager_get(&renderer->material_manager, material_handle);
    if (!material) {
    	if (material_handle != AQUA_MATERIAL_HANDLE_INVALID) {
    		// a handle that was valid once: its material was destroyed but it's still being drawn
    		static bool warned = false;
    		if (!warned) {
    			fprintf(stderr, "[WARNING] [Aqua] [Renderer] Bound destroyed material %zu, using the default material!\n", material_handle);
    			warned = true;
    		}
    	}
    	material = aqua_material_manager_get(&renderer->material_manager, renderer->default_material);
    }

    if (!aqua_renderer_resource_manager_is_shader_valid(&renderer->resource_manager, material->shader)) {
    	renderer->current_shader = AQUA_SHADER_HANDLE_INVALID; // draws get skipped
    	return;
    }
    AquaShader* shader = &renderer->resource_manager.shaders[material->shader];

    aqua_renderer_bind_shader(renderer, material->shader);
    aqua_renderer_bind_texture(renderer, material->texture);

    aqua_shader_set_vec4_uniform(shader->color_uniform_location, material->color);
    aqua_shader_set_float_uniform(shader->color_strength_uniform_location, material->color_strength);

    renderer->current_shader = material->shader;
}

AquaMaterialHandle aqua_renderer_create_material(AquaRenderer* renderer, AquaTextureHandle texture, AquaColor color) {
	return aqua_material_manager_create(&renderer->material_manager, renderer->base_material_shader, texture, color);
}

void aqua_renderer_destroy_material(AquaRenderer* renderer, AquaMaterialHandle material) {
	if (material == renderer->default_material) {
		fprintf(stderr, "[ERROR] [Aqua] [Renderer] The default material can't be destroyed!\n");
		return;
	}
	aqua_material_manager_destroy_material(&renderer->material_manager, material);
}

AquaMaterial* aqua_renderer_get_material(AquaRenderer* renderer, AquaMaterialHandle material) {
	return aqua_material_manager_get(&renderer->material_manager, material);
}

void aqua_renderer_set_material_shader(AquaRenderer* renderer, AquaMaterialHandle material_handle, AquaShaderHandle shader) {
	AquaMaterial* material = aqua_material_manager_get(&renderer->material_manager, material_handle);
	if (!material) {
		fprintf(stderr, "[ERROR] [Aqua] [Renderer] Can't set shader of invalid material %zu!\n", material_handle);
		return;
	}
	if (!aqua_renderer_resource_manager_is_shader_valid(&renderer->resource_manager, shader)) {
		fprintf(stderr, "[ERROR] [Aqua] [Renderer] Can't set material %zu to invalid shader %zu!\n", material_handle, shader);
		return;
	}

	material->shader = shader;
}

void aqua_renderer_bind_camera(AquaRenderer* renderer, AquaCamera* camera) {
    if (camera == NULL) { return; }
    if (camera == renderer->current_camera) { return; }

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

AquaMeshHandle aqua_renderer_create_mesh(AquaRenderer* renderer, AquaVertex* vertices, u32 vertices_count, GLuint* indices, u32 indices_count) {
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

    AquaShader* shader = &renderer->resource_manager.shaders[renderer->current_shader];
    aqua_shader_set_mat4_uniform(shader->view_uniform_location, renderer->current_camera->view);
    aqua_shader_set_mat4_uniform(shader->projection_uniform_location, renderer->current_camera->projection);
    aqua_shader_set_mat4_uniform(shader->model_uniform_location, transform);

    glBindVertexArray(m->vao);
    glDrawElements(GL_TRIANGLES, (GLsizei) indices_count, GL_UNSIGNED_INT, (void*)(uintptr_t)(indices_start * sizeof(GLuint)));
}

void aqua_renderer_draw_static_model(AquaRenderer* renderer, AquaStaticModel* model) {
   	for (usize i = 0; i < model->submeshes_count; i++) {
        AquaStaticModelSubMesh* submesh = &model->submeshes[i];

        aqua_renderer_bind_material(renderer, submesh->material);
        aqua_renderer_draw_submesh(renderer, submesh->mesh, submesh->indices_start, submesh->indices_count, model->transform);
	}
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

	if (renderer->properties.show_properties_window) {
		aqua_renderer_imgui_properties_window(renderer);
	}
	if (renderer->properties.show_performance_window) {
		aqua_renderer_imgui_performance_window(renderer);
	}
}

void aqua_renderer_imgui_performance_window(AquaRenderer* renderer) {
	igBegin("Renderer Performance", &renderer->properties.show_performance_window, ImGuiWindowFlags_None);
	igText("FPS: %0.2f", renderer->fps);
	igText("Deltatime: %0.2f ms", renderer->delta_time * 1000.0);
	igEnd();
}

void aqua_renderer_imgui_properties_window(AquaRenderer* renderer) {
	igBegin("Renderer Properties", &renderer->properties.show_properties_window, ImGuiWindowFlags_None);
	igText("FPS: %0.2f", renderer->fps);
	igText("Deltatime: %0.2f ms", renderer->delta_time * 1000.0);
	igSeparator();
	igText("Video Driver: %s\n", renderer->properties.gl_video_driver);
	igText("Vendor: %s\n", renderer->properties.gl_vender);
	igText("Renderer: %s\n", renderer->properties.gl_renderer);
	igText("OpenGL Version: %s\n",   renderer->properties.gl_opengl_version);
	igText("GLSL Version: %s\n", renderer->properties.gl_glsl_version);
	igSeparator();
	igText("Currently bound shader handle: %zu", renderer->current_shader);
	if (igButton("Rebind Shader", (ImVec2) { 0.0f, 0.0f })) {
        aqua_renderer_rebind_shader(renderer);
	}
	igSeparator();
	if (igCheckbox("Vsync", &renderer->properties.vsync)) { aqua_renderer_set_vsync(renderer, renderer->properties.vsync); }
	igSeparator();
	if (igCheckbox("Rainbow Clear Color", &renderer->properties.rainbow_clear_color)) {
		aqua_renderer_set_clear_color(renderer->properties.clear_color);
	}
	igInputFloat("Rainbow Clear Color Speed", &renderer->properties.rainbow_clear_color_speed, 0.1f, 0.2f, "%0.2f", ImGuiInputTextFlags_None);
	if (igColorEdit3("Clear Color", renderer->properties.clear_color, ImGuiColorEditFlags_None) && !renderer->properties.rainbow_clear_color) {
		aqua_renderer_set_clear_color(renderer->properties.clear_color);
	}
	igSeparator();
	igCheckbox("Performance Window", &renderer->properties.show_performance_window);
	igCheckbox("Show Renderer Resource Manager Properties", &renderer->resource_manager.properties.show_properties_window);
	igCheckbox("Show Material Manager Properties", &renderer->material_manager.properties.show_properties_window);
	igEnd();
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
	aqua_material_manager_destroy(&renderer->material_manager);
	aqua_renderer_resource_manager_destroy(&renderer->resource_manager);
	SDL_GL_DestroyContext(renderer->gl_context);
	free(renderer);

 	#ifndef NDEBUG
    	printf("[INFO] [Aqua] [Renderer] Renderer Destroyed\n");
    #endif
}
