#pragma once

#include <cglm/cglm.h>

#include "utils/base-types.h"
#include "event/event.h"

#define AQUA_CAMERA_DEFAULT_NEAR_PLANE 0.1f
#define AQUA_CAMERA_DEFAULT_FAR_PLANE 1000.0f

typedef struct AquaCameraProperties {
	bool show_properties_window;
} AquaCameraProperties;

typedef struct AquaCamera {
	vec3 position;

	vec3 front;
	vec3 up;

	f32 fov;

	f32 near_plane;
	f32 far_plane;

	mat4 view;
	mat4 projection;

	f32 current_aspect;

	AquaCameraProperties properties;
} AquaCamera;

AquaCamera aqua_camera_create(vec3 position, f32 fov, f32 aspect);
void aqua_camera_update_projection(AquaCamera* camera);
void aqua_camera_update_aspect(AquaCamera* camera, f32 aspect);
void aqua_camera_update(AquaCamera* camera);

void aqua_camera_imgui_update(AquaCamera* camera);
void aqua_camera_imgui_properties_window(AquaCamera* camera);

void aqua_camera_destroy(AquaCamera* camera);
