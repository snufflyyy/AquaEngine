#include "clock.h"

#include <assert.h>
#include <stdio.h>

#include <SDL3/SDL.h>

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include <cimgui.h>
#include <cimgui_impl.h>

AquaClock aqua_clock_create(AquaClockProperties properties) {
	AquaClock clock = {
		.frequency = SDL_GetPerformanceFrequency(),
	};
	(void) aqua_clock_set_properties(&clock, properties);
	aqua_clock_reset(&clock);

	#ifndef NDEBUG
		printf("[INFO] [Aqua] [Clock] Created Clock (%u ticks/s)\n", clock.properties.tick_rate);
	#endif

	return clock;
}

void aqua_clock_reset(AquaClock* clock) {
	assert(clock != NULL);

	clock->last_count = SDL_GetPerformanceCounter();

	clock->delta_time = 0.0;
	clock->time = 0.0;
	clock->frame_count = 0;
	clock->fps = 0.0f;

	clock->lag = 0.0;
	clock->tick_count = 0;
}

void aqua_clock_begin_frame(AquaClock* clock) {
	assert(clock != NULL);

	u64 current_count = SDL_GetPerformanceCounter();
	u64 elapsed_counts = current_count - clock->last_count;
	clock->last_count = current_count;

	f64 frame_time = (f64) elapsed_counts / (f64) clock->frequency;
	if (frame_time > 0.0) { clock->fps = (f32) (1.0 / frame_time); }

	clock->delta_time = (frame_time > AQUA_CLOCK_MAX_DELTA_TIME) ? AQUA_CLOCK_MAX_DELTA_TIME : frame_time;
	clock->time += clock->delta_time;
	clock->frame_count++;

	// the same capped time the frame sees, so ticks never drift from frames
	clock->lag += clock->delta_time;
}

bool aqua_clock_should_tick(const AquaClock* clock) {
	assert(clock != NULL);
	return (clock->lag >= clock->seconds_per_tick);
}

void aqua_clock_end_tick(AquaClock* clock) {
	assert(clock != NULL);

	clock->lag -= clock->seconds_per_tick;
	clock->tick_count++;
}

f64 aqua_clock_get_tick_alpha(const AquaClock* clock) {
	assert(clock != NULL);

	f64 alpha = clock->lag / clock->seconds_per_tick;
	return (alpha > 1.0) ? 1.0 : alpha; // only above 1 if asked before the frame's ticks have run
}

AquaClockProperties aqua_clock_get_properties(const AquaClock* clock) {
	return clock->properties;
}

bool aqua_clock_set_properties(AquaClock* clock, AquaClockProperties properties) {
	if (!clock) { return false; }
	if (properties.tick_rate < AQUA_CLOCK_MIN_TICK_RATE) { properties.tick_rate = AQUA_CLOCK_MIN_TICK_RATE; }
	if (properties.tick_rate > AQUA_CLOCK_MAX_TICK_RATE) { properties.tick_rate = AQUA_CLOCK_MAX_TICK_RATE; }
	clock->properties = properties;
	clock->seconds_per_tick = 1.0 / properties.tick_rate;
	return true;
}

void aqua_clock_set_tick_rate(AquaClock* clock, u32 new_tick_rate) {
	AquaClockProperties properties = aqua_clock_get_properties(clock);
	properties.tick_rate = new_tick_rate;
	(void) aqua_clock_set_properties(clock, properties);
}

AquaClockStats aqua_clock_get_stats(const AquaClock* clock) {
	return (AquaClockStats) {
		.delta_time = clock->delta_time, .time = clock->time,
		.seconds_per_tick = clock->seconds_per_tick,
		.tick_alpha = aqua_clock_get_tick_alpha(clock),
		.frame_count = clock->frame_count, .tick_count = clock->tick_count, .fps = clock->fps,
	};
}

void aqua_clock_imgui_update(AquaClock* clock) {
	if (clock->properties.show_properties_window) {
		aqua_clock_imgui_properties_window(clock);
	}
}

void aqua_clock_imgui_properties_window(AquaClock* clock) {
	AquaClockProperties properties = aqua_clock_get_properties(clock);
	igBegin("Clock Properties", &properties.show_properties_window, ImGuiWindowFlags_None);
	igSeparatorText("Frame");
	igText("FPS: %0.2f", clock->fps);
	igText("Deltatime: %0.2f ms", clock->delta_time * 1000.0);
	igText("Time: %0.2f s", clock->time);
	igText("Frames: %llu", (unsigned long long) clock->frame_count);
	igSeparatorText("Tick");
	// edited as signed so stepping below the minimum can't wrap around to a huge u32
	i32 tick_rate = (i32) properties.tick_rate;
	i32 step = 1;
	i32 fast_step = 5;
	bool changed = false;
	if (igInputScalar("Tick Rate", ImGuiDataType_S32, &tick_rate, &step, &fast_step, "%d", ImGuiInputTextFlags_None)) {
		if (tick_rate < AQUA_CLOCK_MIN_TICK_RATE) { tick_rate = AQUA_CLOCK_MIN_TICK_RATE; }
		properties.tick_rate = (u32) tick_rate;
		changed = true;
	}
	igText("Time Per Tick: %0.2f ms", clock->seconds_per_tick * 1000.0);
	igText("Ticks: %llu", (unsigned long long) clock->tick_count);
	igText("Tick Alpha: %0.2f", aqua_clock_get_tick_alpha(clock));
	igEnd();
	if (changed || properties.show_properties_window != clock->properties.show_properties_window) {
		(void) aqua_clock_set_properties(clock, properties);
	}
}
