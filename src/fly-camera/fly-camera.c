#include "fly-camera.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

AquaFlyCamera* aqua_fly_camera_create(AquaCameraProperties camera_properties, AquaFlyCameraProperties properties) {
	AquaFlyCamera* fly_camera = calloc(1, sizeof(*fly_camera));
	if (!fly_camera) { return NULL; }
	fly_camera->camera = aqua_camera_create(camera_properties);
	if (!fly_camera->camera || !aqua_fly_camera_set_properties(fly_camera, properties)) {
		aqua_fly_camera_destroy(fly_camera);
		return NULL;
	}
	return fly_camera;
}

AquaCamera* aqua_fly_camera_get_camera(AquaFlyCamera* fly_camera) {
	return fly_camera->camera;
}

AquaFlyCameraProperties aqua_fly_camera_get_properties(const AquaFlyCamera* fly_camera) {
	return fly_camera->properties;
}

bool aqua_fly_camera_set_properties(AquaFlyCamera* fly_camera, AquaFlyCameraProperties properties) {
	if (!fly_camera || !isfinite(properties.move_speed) || properties.move_speed < 0.0f
		|| !isfinite(properties.mouse_sensitivity) || properties.mouse_sensitivity < 0.0f) {
		fprintf(stderr, "[ERROR] [Aqua] [Fly Camera] Invalid controller properties!\n");
		return false;
	}
	fly_camera->properties = properties;
	return true;
}

void aqua_fly_camera_reset_input(AquaFlyCamera* fly_camera) {
	fly_camera->forward_pressed = false;
	fly_camera->backward_pressed = false;
	fly_camera->left_pressed = false;
	fly_camera->right_pressed = false;
}

static void fly_camera_mouse_motion(AquaFlyCamera* fly_camera, const AquaEventMouseMotion* motion) {
	AquaCameraProperties camera = aqua_camera_get_properties(fly_camera->camera);
	// Derive angles from the current camera, so edits through its properties never
	// leave a second controller orientation cache out of sync.
	f32 yaw = glm_deg(atan2f(camera.front[2], camera.front[0]));
	f32 pitch = glm_deg(asinf(glm_clamp(camera.front[1], -1.0f, 1.0f)));
	yaw += motion->x_relative * fly_camera->properties.mouse_sensitivity;
	pitch -= motion->y_relative * fly_camera->properties.mouse_sensitivity;
	if (!isfinite(yaw) || !isfinite(pitch)) { return; }
	pitch = glm_clamp(pitch, -89.0f, 89.0f);
	camera.front[0] = cosf(glm_rad(yaw)) * cosf(glm_rad(pitch));
	camera.front[1] = sinf(glm_rad(pitch));
	camera.front[2] = sinf(glm_rad(yaw)) * cosf(glm_rad(pitch));
	(void) aqua_camera_set_properties(fly_camera->camera, camera);
}

void aqua_fly_camera_event(AquaFlyCamera* fly_camera, const AquaEvent* event) {
	if (event->type == AQUA_EVENT_TYPE_MOUSE_MOTION) {
		fly_camera_mouse_motion(fly_camera, &event->mouse_motion);
		return;
	}
	if (event->type != AQUA_EVENT_TYPE_KEY_DOWN && event->type != AQUA_EVENT_TYPE_KEY_UP) { return; }
	bool pressed = event->type == AQUA_EVENT_TYPE_KEY_DOWN;
	switch (event->key.scancode) {
		case AQUA_SCANCODE_W: fly_camera->forward_pressed = pressed; break;
		case AQUA_SCANCODE_S: fly_camera->backward_pressed = pressed; break;
		case AQUA_SCANCODE_A: fly_camera->left_pressed = pressed; break;
		case AQUA_SCANCODE_D: fly_camera->right_pressed = pressed; break;
		default: break;
	}
}

void aqua_fly_camera_update(AquaFlyCamera* fly_camera, f32 delta_time) {
	if (!isfinite(delta_time) || delta_time < 0.0f) { return; }
	f32 distance = fly_camera->properties.move_speed * delta_time;
	if (!isfinite(distance)) { return; }
	AquaCameraProperties camera = aqua_camera_get_properties(fly_camera->camera);
	vec3 forward, right;
	glm_vec3_scale(camera.front, distance, forward);
	glm_vec3_crossn(camera.front, camera.up, right);
	glm_vec3_scale(right, distance, right);
	if (fly_camera->forward_pressed) { glm_vec3_add(camera.position, forward, camera.position); }
	if (fly_camera->backward_pressed) { glm_vec3_sub(camera.position, forward, camera.position); }
	if (fly_camera->left_pressed) { glm_vec3_sub(camera.position, right, camera.position); }
	if (fly_camera->right_pressed) { glm_vec3_add(camera.position, right, camera.position); }
	(void) aqua_camera_set_properties(fly_camera->camera, camera);
}

void aqua_fly_camera_destroy(AquaFlyCamera* fly_camera) {
	if (!fly_camera) { return; }
	aqua_camera_destroy(fly_camera->camera);
	free(fly_camera);
}
