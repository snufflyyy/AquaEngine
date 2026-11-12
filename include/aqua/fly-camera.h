#pragma once
#include "camera.h"
#include "event.h"

#define AQUA_FLY_CAMERA_DEFAULT_MOVE_SPEED 1.0f
#define AQUA_FLY_CAMERA_DEFAULT_MOUSE_SENSITIVITY 0.2f

typedef struct AquaFlyCameraProperties {
	f32 move_speed;
	f32 mouse_sensitivity;
} AquaFlyCameraProperties;
static const AquaFlyCameraProperties AQUA_FLY_CAMERA_DEFAULT_PROPERTIES = {
	.move_speed = AQUA_FLY_CAMERA_DEFAULT_MOVE_SPEED,
	.mouse_sensitivity = AQUA_FLY_CAMERA_DEFAULT_MOUSE_SENSITIVITY,
};

// Owns its camera. The returned camera is borrowed and must not be destroyed separately.
AquaFlyCamera* aqua_fly_camera_create(AquaCameraProperties camera_properties, AquaFlyCameraProperties properties);
void aqua_fly_camera_destroy(AquaFlyCamera* fly_camera);
AquaCamera* aqua_fly_camera_get_camera(AquaFlyCamera* fly_camera);
AquaFlyCameraProperties aqua_fly_camera_get_properties(const AquaFlyCamera* fly_camera);
bool aqua_fly_camera_set_properties(AquaFlyCamera* fly_camera, AquaFlyCameraProperties properties);
void aqua_fly_camera_event(AquaFlyCamera* fly_camera, const AquaEvent* event);
void aqua_fly_camera_update(AquaFlyCamera* fly_camera, f32 delta_time);
// Clear held keys when control/capture is disabled.
void aqua_fly_camera_reset_input(AquaFlyCamera* fly_camera);
