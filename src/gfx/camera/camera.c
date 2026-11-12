#include "camera.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include <cimgui.h>

static bool camera_properties_valid(const AquaCameraProperties* properties) {
	for (int i = 0; i < 3; i++) {
		if (!isfinite(properties->position[i]) || !isfinite(properties->front[i]) || !isfinite(properties->up[i])) { return false; }
	}
	if (!isfinite(properties->fov) || properties->fov <= 0.0f || properties->fov >= 180.0f
		|| !isfinite(properties->aspect) || properties->aspect <= 0.0f
		|| !isfinite(properties->near_plane) || properties->near_plane <= 0.0f
		|| !isfinite(properties->far_plane) || properties->far_plane <= properties->near_plane) { return false; }
	vec3 front = { properties->front[0], properties->front[1], properties->front[2] };
	vec3 up = { properties->up[0], properties->up[1], properties->up[2] };
	f32 front_length = glm_vec3_norm(front);
	f32 up_length = glm_vec3_norm(up);
	if (!isfinite(front_length) || !isfinite(up_length) || front_length < 1e-6f || up_length < 1e-6f) { return false; }
	glm_vec3_normalize(front);
	glm_vec3_normalize(up);
	vec3 right;
	glm_vec3_cross(front, up, right);
	return glm_vec3_norm2(right) > 1e-12f;
}

AquaCamera* aqua_camera_create(AquaCameraProperties properties) {
	AquaCamera* camera = calloc(1, sizeof(*camera));
	if (!camera) { return NULL; }

	if (!aqua_camera_set_properties(camera, properties)) {
		free(camera);
		return NULL;
	}

	return camera;
}

AquaCameraProperties aqua_camera_get_properties(const AquaCamera* camera) {
	return camera->properties;
}

bool aqua_camera_set_properties(AquaCamera* camera, AquaCameraProperties properties) {
	if (!camera || !camera_properties_valid(&properties)) {
		fprintf(stderr, "[ERROR] [Aqua] [Camera] Invalid camera properties!\n");
		return false;
	}

	glm_vec3_normalize(properties.front);
	glm_vec3_normalize(properties.up);

	mat4 view;
	// build the basis at the origin so position + front cannot lose direction
	// precision at large coordinates. Then apply the translated eye position.
	vec3 origin = { 0.0f, 0.0f, 0.0f };
	glm_lookat(origin, properties.front, properties.up, view);
	for (int r = 0; r < 3; r++) {
		view[3][r] = -(view[0][r] * properties.position[0] + view[1][r] * properties.position[1] + view[2][r] * properties.position[2]);
	}

	mat4 projection;
	glm_perspective(glm_rad(properties.fov), properties.aspect, properties.near_plane, properties.far_plane, projection);
	for (int c = 0; c < 4; c++) {
		for (int r = 0; r < 4; r++) {
			if (!isfinite(view[c][r]) || !isfinite(projection[c][r])) {
				fprintf(stderr, "[ERROR] [Aqua] [Camera] Properties produced non-finite matrices!\n");
				return false;
			}
		}
	}

	camera->properties = properties;
	glm_mat4_copy(view, camera->view);
	glm_mat4_copy(projection, camera->projection);

	return true;
}

bool aqua_camera_update_aspect(AquaCamera* camera, f32 aspect) {
	AquaCameraProperties properties = aqua_camera_get_properties(camera);
	properties.aspect = aspect;

	return aqua_camera_set_properties(camera, properties);
}

void aqua_camera_imgui_update(AquaCamera* camera) {
	if (!camera) { return; }
	if (camera->properties.show_properties_window) { aqua_camera_imgui_properties_window(camera); }
}

void aqua_camera_imgui_properties_window(AquaCamera* camera) {
	AquaCameraProperties properties = aqua_camera_get_properties(camera);
	igBegin("Camera Properties", &properties.show_properties_window, ImGuiWindowFlags_None);
	bool changed = igInputFloat3("Position", properties.position, "%0.2f", ImGuiInputTextFlags_None);
	changed |= igInputFloat("FOV", &properties.fov, 1.0f, 5.0f, "%0.2f", ImGuiInputTextFlags_None);
	changed |= igInputFloat("Near Plane", &properties.near_plane, 0.01f, 0.05f, "%0.2f", ImGuiInputTextFlags_None);
	changed |= igInputFloat("Far Plane", &properties.far_plane, 1.0f, 5.0f, "%0.2f", ImGuiInputTextFlags_None);
	igEnd();
	if (changed || properties.show_properties_window != camera->properties.show_properties_window) {
		(void) aqua_camera_set_properties(camera, properties);
	}
}

void aqua_camera_destroy(AquaCamera* camera) {
	free(camera);
}
