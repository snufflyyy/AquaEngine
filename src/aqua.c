#include "aqua.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include <glad/glad.h>
#include <SDL3/SDL.h>

#include "cimgui.h"
#include "event/event.h"
#include "gfx/renderer/renderer.h"
#include "gfx/window/window.h"
#include "input/input.h"
#include "project/project.h"
#include "tick/tick.h"

AquaEngineContext* aqua_engine_context_create(AquaProject* project, AquaEngineContextInitializeProperties properties) {
	AquaEngineContext* aqua_context = (AquaEngineContext*) malloc(sizeof(AquaEngineContext));
	if (!aqua_context) {
		fprintf(stderr, "[ERROR] [Aqua] Failed to allocate memory for aqua context!\n");
		goto error;
	}

	aqua_context->project = *project;

	aqua_context->tick_handler = aqua_tick_handler_create(properties.tick_handler_properties.tick_rate);

	if (!SDL_Init(SDL_INIT_VIDEO)) {
		fprintf(stderr, "[ERROR] [Aqua] Failed to initialize SDL!\n");
		goto error;
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
 	#ifdef __APPLE__
   		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    #else
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    #endif
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

	aqua_context->window = aqua_window_create(properties.window_properties);
	if (!aqua_context->window) {
		fprintf(stderr, "[ERROR] [Aqua] Failed to create window!\n");
		goto error;
	}

	aqua_context->renderer = aqua_renderer_create(properties.renderer_create_properties, aqua_context->window);
	if (!aqua_context->renderer) {
		fprintf(stderr, "[ERROR] [Aqua] Failed to create renderer!\n");
		goto error;
	}

	aqua_context->input_manager = aqua_input_manager_create(aqua_context->window);

	aqua_context->properties = (AquaContextProperties) {
		.show_properties_window = properties.show_properties_window,
	};

	return aqua_context;

error:
	if (aqua_context->renderer) { aqua_renderer_destroy(aqua_context->renderer); }
	if (aqua_context->window) { aqua_window_destroy(aqua_context->window); }
	SDL_Quit();
	if (aqua_context) { free(aqua_context); }
	return NULL;
}

void aqua_engine_context_event(AquaEngineContext* aqua_context, AquaEvent* event) {
	assert(aqua_context != NULL);
	assert(event != NULL);

	switch (event->type) {
		case AQUA_EVENT_TYPE_EXIT: aqua_context->running = false; break;
		default: break;
	}

	aqua_window_event(aqua_context->window, event);

	aqua_context->renderer->properties.imgui_process_input = !aqua_context->input_manager.properties.mouse_captured;
	aqua_renderer_event(aqua_context->renderer, event);
}

void aqua_engine_context_tick(AquaEngineContext* aqua_context) {
	assert(aqua_context != NULL);

}

void aqua_engine_context_update(AquaEngineContext* aqua_context) {
	assert(aqua_context != NULL);

	aqua_renderer_update(aqua_context->renderer);

	aqua_engine_imgui_update(aqua_context);
}

void aqua_engine_context_destroy(AquaEngineContext* aqua_context) {
	assert(aqua_context != NULL);

	aqua_input_manager_destroy(&aqua_context->input_manager);
	aqua_renderer_destroy(aqua_context->renderer);
	aqua_window_destroy(aqua_context->window);
	SDL_Quit();
	free(aqua_context);
}

void aqua_engine_imgui_update(AquaEngineContext* aqua_context) {
	aqua_tick_handler_imgui_update(&aqua_context->tick_handler);
	aqua_window_imgui_update(aqua_context->window);
	aqua_renderer_imgui_update(aqua_context->renderer);
	aqua_input_manager_imgui_update(&aqua_context->input_manager);

	if (aqua_context->properties.show_properties_window) {
		aqua_engine_imgui_properties_window(aqua_context);
	}
}

void aqua_engine_imgui_properties_window(AquaEngineContext* aqua_context) {
	igBegin("Aqua Properties", &aqua_context->properties.show_properties_window, ImGuiWindowFlags_None);
	igSeparatorText("Project");
	igText("Name: %s\n", aqua_context->project.name);
	igText("Creator: %s\n", aqua_context->project.creator);
	igSeparator();
	igCheckbox("Show Tick Handler Properties", &aqua_context->tick_handler.properties.show_properties_window);
	igCheckbox("Show Window Properties", &aqua_context->window->properties.show_properties_window);
	igCheckbox("Show Renderer Properties", &aqua_context->renderer->properties.show_properties_window);
	igCheckbox("Show Input Manager Properties", &aqua_context->input_manager.properties.show_properties_window);
	igEnd();
}

void aqua_engine_run(AquaEngineContext* aqua_context) {
	aqua_context->project.initialize(aqua_context);

	aqua_context->running = true;
	while (aqua_context->running) {
		AquaEvent event;
		while (aqua_event_poll(&event)) {
			aqua_engine_context_event(aqua_context, &event);
			aqua_context->project.event(aqua_context, &event);
		}

		aqua_tick_handler_update(&aqua_context->tick_handler);
		if (aqua_tick_handler_should_tick(&aqua_context->tick_handler)) {
			aqua_engine_context_tick(aqua_context);
			aqua_context->project.tick(aqua_context);

			aqua_tick_handler_update_lag(&aqua_context->tick_handler);
		}

		aqua_renderer_imgui_begin(aqua_context->renderer);

		aqua_engine_context_update(aqua_context);
		aqua_context->project.update(aqua_context);

		aqua_context->project.render(aqua_context);

		aqua_renderer_imgui_end(aqua_context->renderer);

		aqua_renderer_imgui_draw(aqua_context->renderer);

		aqua_window_swap(aqua_context->window);
	}

	aqua_context->project.destroy(aqua_context);
}
