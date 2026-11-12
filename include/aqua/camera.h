#pragma once
#include "types.h"

#define AQUA_CAMERA_DEFAULT_NEAR_PLANE 0.1f
#define AQUA_CAMERA_DEFAULT_FAR_PLANE 1000.0f

typedef struct AquaCameraProperties {
	vec3 position;
	vec3 front;
	vec3 up;
	f32 fov; // degrees, between 0 and 180.
	f32 near_plane, far_plane;
	f32 aspect;
	bool show_properties_window;
} AquaCameraProperties;
static const AquaCameraProperties AQUA_CAMERA_DEFAULT_PROPERTIES = {
	.front = {0.0f, 0.0f, -1.0f}, .up = {0.0f, 1.0f, 0.0f},
	.fov = 70.0f, .near_plane = AQUA_CAMERA_DEFAULT_NEAR_PLANE,
	.far_plane = AQUA_CAMERA_DEFAULT_FAR_PLANE, .aspect = 16.0f / 9.0f,
};

// Application-owned. Unbind from the renderer before destroying.
AquaCamera* aqua_camera_create(AquaCameraProperties properties);
void aqua_camera_destroy(AquaCamera* camera);
AquaCameraProperties aqua_camera_get_properties(const AquaCamera* camera);
// Rejects non-finite/degenerate values; normalizes direction vectors and rebuilds matrices.
bool aqua_camera_set_properties(AquaCamera* camera, AquaCameraProperties properties);
bool aqua_camera_update_aspect(AquaCamera* camera, f32 aspect);
