#include "aqua.h"
#include "gfx/renderer/renderer.h"
#include "event/event.h"

#include <stdio.h>
#include <stdlib.h>

AquaEngineContext* aqua_engine_context_create(AquaProject* project, AquaEngineContextInitializeProperties properties) {
	if (!project) { return NULL; }

	bool input_initialized = false;

	AquaEngineContext* context = calloc(1, sizeof(*context));
	if (!context) { return NULL; }

	context->project = project;
	context->running = true;

	if (!SDL_Init(SDL_INIT_VIDEO)) {
		fprintf(stderr, "[ERROR] [Aqua] Failed to initialize SDL: %s\n", SDL_GetError());
		goto error;
	}

	context->clock = aqua_clock_create(properties.clock_properties);

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

	context->window = aqua_window_create(properties.window_properties);
	if (!context->window) { goto error; }

	context->renderer = aqua_renderer_create(properties.renderer_properties, context->window);
	if (!context->renderer) { goto error; }

	context->input_manager = aqua_input_manager_create(context->window);
	input_initialized = true;

	if (!aqua_input_set_properties(context, properties.input_properties)) { goto error; }

	(void) aqua_engine_context_set_properties(context, properties.context_properties);
	if (project->initialize && !project->initialize(context)) {
		fprintf(stderr, "[ERROR] [Aqua] Failed to initialize project '%s'!\n", project->name ? project->name : "");
		goto error;
	}

	return context;

error:
	if (input_initialized) { aqua_input_manager_destroy(&context->input_manager); }
	if (context->renderer) { aqua_renderer_destroy(context->renderer); }
	if (context->window) { aqua_window_destroy(context->window); }
	SDL_Quit();
	free(context);
	return NULL;
}

AquaRenderer* aqua_engine_context_get_renderer(AquaEngineContext* context) { return context->renderer; }
AquaWindow* aqua_engine_context_get_window(AquaEngineContext* context) { return context->window; }
void* aqua_engine_context_get_project_data(AquaEngineContext* context) { return context->project->project_data_ptr; }
f64 aqua_engine_context_get_delta_time(const AquaEngineContext* context) { return context->clock.delta_time; }
void aqua_engine_context_request_exit(AquaEngineContext* context) { context->running = false; }

AquaContextProperties aqua_engine_context_get_properties(const AquaEngineContext* context) {
	return context->properties;
}

bool aqua_engine_context_set_properties(AquaEngineContext* context, AquaContextProperties properties) {
	if (!context) { return false; }
	context->properties = properties;
	return true;
}

AquaClockProperties aqua_engine_context_get_clock_properties(const AquaEngineContext* context) {
	return aqua_clock_get_properties(&context->clock);
}

bool aqua_engine_context_set_clock_properties(AquaEngineContext* context, AquaClockProperties properties) {
	return context && aqua_clock_set_properties(&context->clock, properties);
}

AquaClockStats aqua_engine_context_get_clock_stats(const AquaEngineContext* context) {
	return aqua_clock_get_stats(&context->clock);
}

AquaInputManagerProperties aqua_input_get_properties(const AquaEngineContext* context) {
	return context->input_manager.properties;
}

bool aqua_input_set_properties(AquaEngineContext* context, AquaInputManagerProperties properties) {
	return context && aqua_input_manager_set_properties(&context->input_manager, properties);
}

bool aqua_input_is_mouse_captured(const AquaEngineContext* context) {
	return aqua_input_get_properties(context).mouse_captured;
}

bool aqua_input_set_mouse_captured(AquaEngineContext* context, bool captured) {
	AquaInputManagerProperties properties = aqua_input_get_properties(context);
	properties.mouse_captured = captured;
	return aqua_input_set_properties(context, properties);
}

void aqua_engine_context_event(AquaEngineContext* context, const AquaEvent* event) {
	if (event->type == AQUA_EVENT_TYPE_EXIT) { aqua_engine_context_request_exit(context); }
	aqua_window_event(context->window, event);
	aqua_renderer_event(context->renderer, event);
}

void aqua_engine_context_update(AquaEngineContext* context) {
	aqua_renderer_update(context->renderer, context->clock.time);
}

void aqua_engine_context_destroy(AquaEngineContext* context) {
	if (!context) { return; }
	if (context->project->destroy) { context->project->destroy(context); }
	aqua_input_manager_destroy(&context->input_manager);
	aqua_renderer_destroy(context->renderer);
	aqua_window_destroy(context->window);
	SDL_Quit();
	free(context);
}

void aqua_engine_run(AquaEngineContext* context) {
	aqua_clock_reset(&context->clock);

	while (context->running) {
		AquaEvent event;
		while (aqua_event_poll(&event, context->renderer, !aqua_input_is_mouse_captured(context))) {
			aqua_engine_context_event(context, &event);
			if (context->project->event) { context->project->event(context, &event); }
		}
		if (!context->running) { break; }

		aqua_clock_begin_frame(&context->clock);
		while (context->running && aqua_clock_should_tick(&context->clock)) {
			if (context->project->tick) { context->project->tick(context); }
			aqua_clock_end_tick(&context->clock);
		}
		if (!context->running) { break; }

		aqua_engine_context_update(context);

		aqua_renderer_imgui_begin(context->renderer);
		aqua_engine_imgui_update(context);

		if (context->project->update) { context->project->update(context); }
		if (context->running && context->project->render) { context->project->render(context); }

		aqua_renderer_imgui_end(context->renderer);
		aqua_renderer_imgui_draw(context->renderer);

		aqua_window_swap(context->window);
	}
}
