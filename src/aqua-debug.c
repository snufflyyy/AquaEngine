#include "aqua.h"
#include "gfx/renderer/renderer.h"

void aqua_engine_imgui_update(AquaEngineContext* context) {
	if (context->project->imgui_update) { context->project->imgui_update(context); }
	aqua_clock_imgui_update(&context->clock);
	aqua_window_imgui_update(context->window);
	aqua_renderer_imgui_update(context->renderer);
	aqua_input_manager_imgui_update(&context->input_manager);
	if (context->properties.show_properties_window) { aqua_engine_imgui_properties_window(context); }
}

void aqua_engine_imgui_properties_window(AquaEngineContext* context) {
	AquaContextProperties properties = aqua_engine_context_get_properties(context);
	AquaClockProperties clock = aqua_engine_context_get_clock_properties(context);
	AquaWindowProperties window = aqua_window_get_properties(context->window);
	AquaRendererProperties renderer = aqua_renderer_get_properties(context->renderer);
	AquaInputManagerProperties input = aqua_input_get_properties(context);
	igBegin("Aqua Properties", &properties.show_properties_window, ImGuiWindowFlags_None);
	igSeparatorText("Project");
	igText("Name: %s", context->project->name ? context->project->name : "");
	igText("Creator: %s", context->project->creator ? context->project->creator : "");
	igCheckbox("Show Project Properties", &context->project->show_properties_window);
	igSeparatorText("Engine");
	bool clock_changed = igCheckbox("Show Clock Properties", &clock.show_properties_window);
	bool window_changed = igCheckbox("Show Window Properties", &window.show_properties_window);
	bool renderer_changed = igCheckbox("Show Renderer Properties", &renderer.show_properties_window);
	bool input_changed = igCheckbox("Show Input Manager Properties", &input.show_properties_window);
	igEnd();
	if (properties.show_properties_window != context->properties.show_properties_window) {
		(void) aqua_engine_context_set_properties(context, properties);
	}
	if (clock_changed) { (void) aqua_engine_context_set_clock_properties(context, clock); }
	if (window_changed) { (void) aqua_window_set_properties(context->window, window); }
	if (renderer_changed) { (void) aqua_renderer_set_properties(context->renderer, renderer); }
	if (input_changed) { (void) aqua_input_set_properties(context, input); }
}
