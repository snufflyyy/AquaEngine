#pragma once
#include <aqua/clock.h>

typedef struct AquaClock {
	AquaClockProperties properties;
	u64 frequency, last_count;
	f64 delta_time, time;
	u64 frame_count;
	f32 fps;
	f64 seconds_per_tick, lag;
	u64 tick_count;
} AquaClock;

AquaClock aqua_clock_create(AquaClockProperties properties);
AquaClockProperties aqua_clock_get_properties(const AquaClock* clock);
bool aqua_clock_set_properties(AquaClock* clock, AquaClockProperties properties);
AquaClockStats aqua_clock_get_stats(const AquaClock* clock);
void aqua_clock_reset(AquaClock* clock);
void aqua_clock_begin_frame(AquaClock* clock);
bool aqua_clock_should_tick(const AquaClock* clock);
void aqua_clock_end_tick(AquaClock* clock);
f64 aqua_clock_get_tick_alpha(const AquaClock* clock);
void aqua_clock_set_tick_rate(AquaClock* clock, u32 tick_rate);
void aqua_clock_imgui_update(AquaClock* clock);
void aqua_clock_imgui_properties_window(AquaClock* clock);
