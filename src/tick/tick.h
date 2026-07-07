#pragma once

#include "utils/base-types.h"

typedef struct AquaTickHandlerProperties {
	u32 tick_rate;
	bool show_properties_window;
} AquaTickHandlerProperties;

static const AquaTickHandlerProperties AQUA_TICK_HANDLER_DEFAULT_PROPERTIES = {
	.tick_rate = 64,
	.show_properties_window = false,
};

typedef struct AquaTickHandler {
	AquaTickHandlerProperties properties;

	f64 ms_per_tick;

	u64 last_count;
	f64 lag;

	f64 frequency;
} AquaTickHandler;

AquaTickHandler aqua_tick_handler_create(u32 tick_rate);

void aqua_tick_handler_set_tick_rate(AquaTickHandler* tick_handler, u32 new_tick_rate);

void aqua_tick_handler_imgui_update(AquaTickHandler* tick_handler);
void aqua_tick_handler_imgui_properties_window(AquaTickHandler* tick_handler);

void aqua_tick_handler_update(AquaTickHandler* tick_handler);
bool aqua_tick_handler_should_tick(AquaTickHandler* tick_handler);
void aqua_tick_handler_update_lag(AquaTickHandler* tick_handler);
