#include "fly-camera.h"
#include "gfx/camera/camera.h"

AquaFlyCamera aqua_fly_camera_create(vec3 position, f32 fov, f32 aspect) {
    AquaFlyCamera fly_camera = {0};

    fly_camera.camera = aqua_camera_create(position, fov, aspect);
    fly_camera.camera_pitch = 0.0f;
    fly_camera.camera_yaw = -90.0f;
    fly_camera.camera_move_speed = AQUA_FLY_CAMERA_DEFAULT_MOVE_SPEED;
    fly_camera.camera_mouse_sensitivity = AQUA_FLY_CAMERA_DEFAULT_MOUSE_SENSITIVITY;

    return fly_camera;
}

void aqua_fly_camera_event(AquaFlyCamera* fly_camera, AquaEvent* event) {
    switch (event->type) {
		case AQUA_EVENT_TYPE_KEY_DOWN: {
			switch (event->key.scancode) {
				case AQUA_SCANCODE_W: { fly_camera->forward_pressed = true; } break;
				case AQUA_SCANCODE_A: { fly_camera->left_pressed = true; } break;
				case AQUA_SCANCODE_S: { fly_camera->backward_pressed = true; } break;
				case AQUA_SCANCODE_D: { fly_camera->right_pressed = true; } break;
				default: break;
			}
		} break;
		case AQUA_EVENT_TYPE_KEY_UP: {
		    switch (event->key.scancode) {
				case AQUA_SCANCODE_W: { fly_camera->forward_pressed = false; } break;
				case AQUA_SCANCODE_A: { fly_camera->left_pressed = false; } break;
				case AQUA_SCANCODE_S: { fly_camera->backward_pressed = false; } break;
				case AQUA_SCANCODE_D: { fly_camera->right_pressed = false; } break;
				default: break;
			}
		} break;
	    case AQUA_EVENT_TYPE_MOUSE_MOTION: {
				fly_camera->camera_yaw += (event->mouse_motion.x_relative * fly_camera->camera_mouse_sensitivity);
				fly_camera->camera_pitch -= (event->mouse_motion.y_relative * fly_camera->camera_mouse_sensitivity);

				if (fly_camera->camera_pitch > 89.0f) { fly_camera->camera_pitch = 89.0f; }
				if (fly_camera->camera_pitch < -89.0f) { fly_camera->camera_pitch = -89.0f; }

				vec3 direction = {
	                cosf(glm_rad(fly_camera->camera_yaw)) * cosf(glm_rad(fly_camera->camera_pitch)),
	                sinf(glm_rad(fly_camera->camera_pitch)),
	                sinf(glm_rad(fly_camera->camera_yaw)) * cosf(glm_rad(fly_camera->camera_pitch)),
	            };
	            glm_normalize(direction);
	            glm_vec3_copy(direction, fly_camera->camera.front);
		} break;
		default: break;
    }
}

void aqua_fly_camera_update(AquaFlyCamera* fly_camera, f32 deltatime) {
   	f32 move_speed_dt = fly_camera->camera_move_speed * deltatime;
    if (fly_camera->forward_pressed) {
        vec3 forward;
        glm_vec3_scale(fly_camera->camera.front, move_speed_dt, forward);
        glm_vec3_add(fly_camera->camera.position, forward, fly_camera->camera.position);
    }
    if (fly_camera->backward_pressed) {
        vec3 backward;
        glm_vec3_scale(fly_camera->camera.front, move_speed_dt, backward);
        glm_vec3_sub(fly_camera->camera.position, backward, fly_camera->camera.position);
    }
    if (fly_camera->left_pressed) {
        vec3 left;
        glm_vec3_crossn(fly_camera->camera.front, fly_camera->camera.up, left);
        glm_vec3_scale(left, move_speed_dt, left);
        glm_vec3_sub(fly_camera->camera.position, left, fly_camera->camera.position);
    }
    if (fly_camera->right_pressed) {
        vec3 right;
        glm_vec3_crossn(fly_camera->camera.front, fly_camera->camera.up, right);
        glm_vec3_scale(right, move_speed_dt, right);
        glm_vec3_add(fly_camera->camera.position, right, fly_camera->camera.position);
    }

    aqua_camera_update(&fly_camera->camera);
}

void aqua_fly_camera_destroy(AquaFlyCamera* fly_camera) {
    aqua_camera_destroy(&fly_camera->camera);
}
