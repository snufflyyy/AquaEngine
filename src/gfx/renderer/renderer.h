#pragma once

#include <aqua/renderer.h>
#include <aqua/debug.h>
#include "resource-manager.h"
#include "gfx/material/material-manager.h"
#include "gfx/model/model-manager.h"
#include "gfx/window/window.h"
#include "gfx/camera/camera.h"
#include "event/event.h"
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include <cimgui.h>
#include <cimgui_impl.h>

struct AquaRenderer {
	SDL_GLContext gl_context;
	AquaRendererResourceManager resource_manager;
	AquaMaterialManager material_manager;
	AquaModelManager model_manager;
	AquaCamera* current_camera;
	AquaShaderHandle current_shader;
	AquaShaderHandle base_material_shader;
	AquaTextureHandle default_texture;
	AquaMaterialHandle default_material;
	ImGuiContext* imgui_context;
	bool started_imgui_frame;
	bool imgui_process_input;
	bool properties_initialized;
	AquaRendererProperties properties;
	AquaRendererInfo info;
};

bool aqua_renderer_apply_properties(AquaRenderer* renderer, AquaRendererProperties properties, bool force);
void aqua_renderer_event(AquaRenderer* renderer, const AquaEvent* event);
void aqua_renderer_imgui_native_event(AquaRenderer* renderer, const SDL_Event* event);
void aqua_renderer_update(AquaRenderer* renderer, f64 time);
void aqua_renderer_imgui_begin(AquaRenderer* renderer);
void aqua_renderer_imgui_update(AquaRenderer* renderer);
void aqua_renderer_imgui_properties_window(AquaRenderer* renderer);
void aqua_renderer_imgui_end(AquaRenderer* renderer);
void aqua_renderer_imgui_draw(AquaRenderer* renderer);
