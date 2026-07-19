#include "tick.h"

#include <assert.h>

#include <SDL3/SDL.h>
#include <stdio.h>
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include <cimgui.h>
#include <cimgui_impl.h>

AquaTickHandler aqua_tick_handler_create(u32 tick_rate) {
	if (tick_rate == 0) { tick_rate = 1; }

	AquaTickHandler tick_handler = {
		.properties = { .tick_rate = tick_rate },
		.ms_per_tick = 1.0 / tick_rate,
		.last_count = SDL_GetPerformanceCounter(),
		.lag = 0.0,
		.frequency = SDL_GetPerformanceFrequency(),
	};

	return tick_handler;
}

void aqua_tick_handler_set_tick_rate(AquaTickHandler* tick_handler, u32 new_tick_rate) {
	if (new_tick_rate == 0) { new_tick_rate = 1; }

	tick_handler->properties.tick_rate = new_tick_rate;
	tick_handler->ms_per_tick = 1.0 / new_tick_rate;
}

void aqua_tick_handler_update(AquaTickHandler* tick_handler) {
	assert(tick_handler != NULL);

	u64 current_count = SDL_GetPerformanceCounter();
	u64 elapsed_counts = current_count - tick_handler->last_count;
	tick_handler->last_count = current_count;

	f64 elapsed_time = (f64) elapsed_counts / tick_handler->frequency;
	tick_handler->lag += elapsed_time;
}

bool aqua_tick_handler_should_tick(AquaTickHandler* tick_handler) {
	assert(tick_handler != NULL);
	return (tick_handler->lag >= tick_handler->ms_per_tick);
}

void aqua_tick_handler_update_lag(AquaTickHandler* tick_handler) {
	assert(tick_handler != NULL);
	tick_handler->lag -= tick_handler->ms_per_tick;
}

void aqua_tick_handler_imgui_update(AquaTickHandler* tick_handler) {
	if (tick_handler->properties.show_properties_window) {
		aqua_tick_handler_imgui_properties_window(tick_handler);
	}
}

void aqua_tick_handler_imgui_properties_window(AquaTickHandler* tick_handler) {
	igBegin("Tick Handler Properties", &tick_handler->properties.show_properties_window, ImGuiWindowFlags_None);
	u32 step = 1;
	u32 fast_step = 5;
	if (igInputScalar("Tick Rate", ImGuiDataType_U32, &tick_handler->properties.tick_rate, &step, &fast_step, "%u", ImGuiInputTextFlags_None)) {
		tick_handler->ms_per_tick = 1.0 / tick_handler->properties.tick_rate;
	}
	igEnd();
}
