#include "renderer.h"

#include <math.h>
#include <stdio.h>

static bool renderer_properties_valid(const AquaRendererProperties properties) {
	if (!isfinite(properties.rainbow_clear_color_speed) || properties.rainbow_clear_color_speed < 0.0f) { return false; }
	for (int i = 0; i < 4; i++) {
		if (!isfinite(properties.clear_color[i])) { return false; }
	}

	return true;
}

AquaRendererProperties aqua_renderer_get_properties(const AquaRenderer* renderer) {
	return renderer->properties;
}

AquaRendererInfo aqua_renderer_get_info(const AquaRenderer* renderer) {
	return renderer->info;
}

bool aqua_renderer_apply_properties(AquaRenderer* renderer, AquaRendererProperties properties, bool force) {
	if (!renderer || !renderer_properties_valid(properties)) {
		fprintf(stderr, "[ERROR] [Aqua] [Renderer] Invalid renderer properties!\n");
		return false;
	}

	if (force || properties.vsync != renderer->properties.vsync) {
		if (!SDL_GL_SetSwapInterval(properties.vsync ? 1 : 0)) {
			fprintf(stderr, "[ERROR] [Aqua] [Renderer] Failed to set swap interval: %s\n", SDL_GetError());
			return false;
		}
	}

	if (force || properties.wireframe != renderer->properties.wireframe) {
		glPolygonMode(GL_FRONT_AND_BACK, properties.wireframe ? GL_LINE : GL_FILL);
	}

	if (!properties.rainbow_clear_color && (force || renderer->properties.rainbow_clear_color
		|| !glm_vec4_eqv(properties.clear_color, renderer->properties.clear_color))) {
		glClearColor(properties.clear_color[0], properties.clear_color[1], properties.clear_color[2], properties.clear_color[3]);
	}

	renderer->properties = properties;
	renderer->properties_initialized = true;

	return true;
}

bool aqua_renderer_set_properties(AquaRenderer* renderer, AquaRendererProperties properties) {
	return aqua_renderer_apply_properties(renderer, properties, renderer && !renderer->properties_initialized);
}

bool aqua_renderer_set_vsync(AquaRenderer* renderer, bool value) {
	AquaRendererProperties properties = aqua_renderer_get_properties(renderer);
	properties.vsync = value;
	return aqua_renderer_set_properties(renderer, properties);
}

bool aqua_renderer_set_wireframe(AquaRenderer* renderer, bool value) {
	AquaRendererProperties properties = aqua_renderer_get_properties(renderer);
	properties.wireframe = value;
	return aqua_renderer_set_properties(renderer, properties);
}

bool aqua_renderer_set_clear_color(AquaRenderer* renderer, AquaColor color) {
	AquaRendererProperties properties = aqua_renderer_get_properties(renderer);
	glm_vec4_copy(color, properties.clear_color);
	return aqua_renderer_set_properties(renderer, properties);
}
