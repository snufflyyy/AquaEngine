#pragma once
#include "types.h"

#define AQUA_CLOCK_MIN_TICK_RATE 1
#define AQUA_CLOCK_MAX_TICK_RATE 1000
#define AQUA_CLOCK_MAX_DELTA_TIME 0.25

typedef struct AquaClockProperties {
	u32 tick_rate;
	bool show_properties_window;
} AquaClockProperties;
static const AquaClockProperties AQUA_CLOCK_DEFAULT_PROPERTIES = { .tick_rate = 64 };

typedef struct AquaClockStats {
	f64 delta_time;
	f64 time;
	f64 seconds_per_tick;
	f64 tick_alpha;
	u64 frame_count;
	u64 tick_count;
	f32 fps;
} AquaClockStats;

AquaClockProperties aqua_engine_context_get_clock_properties(const AquaEngineContext* context);
// Tick rate is clamped to the documented limits. Accumulated time is preserved.
bool aqua_engine_context_set_clock_properties(AquaEngineContext* context, AquaClockProperties properties);
AquaClockStats aqua_engine_context_get_clock_stats(const AquaEngineContext* context);
