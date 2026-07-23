#pragma once

#include "event/event.h"
#include "gfx/camera/camera.h"
#include "utils/base-types.h"

//
// this is more of a debug tool
//

#define AQUA_FLY_CAMERA_DEFAULT_MOVE_SPEED 1.0f
#define AQUA_FLY_CAMERA_DEFAULT_MOUSE_SENSITIVITY 0.2f

typedef struct AquaFlyCamera {
   	AquaCamera camera;
	f32 camera_yaw, camera_pitch;
	f32 camera_move_speed;
	f32 camera_mouse_sensitivity;

	bool forward_pressed;
	bool backward_pressed;
	bool left_pressed;
	bool right_pressed;
} AquaFlyCamera;

AquaFlyCamera aqua_fly_camera_create(vec3 position, f32 fov, f32 aspect);
void aqua_fly_camera_event(AquaFlyCamera* fly_camera, AquaEvent* event);
void aqua_fly_camera_update(AquaFlyCamera* fly_camera, f32 deltatime);
void aqua_fly_camera_destroy(AquaFlyCamera* fly_camera);
