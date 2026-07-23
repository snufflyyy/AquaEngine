#include "camera.h"
#include "cglm/util.h"

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include <cimgui.h>
#include <cimgui_impl.h>

AquaCamera aqua_camera_create(vec3 position, f32 fov, f32 aspect) {
	AquaCamera camera = {0};

	glm_vec3_copy(position, camera.position);

	glm_vec3_copy((vec3) { 0.0f, 0.0f, -1.0f }, camera.front);
	glm_vec3_copy((vec3) { 0.0f, 1.0f, 0.0f }, camera.up);

	camera.fov = fov;

	camera.near_plane = AQUA_CAMERA_DEFAULT_NEAR_PLANE;
	camera.far_plane = AQUA_CAMERA_DEFAULT_FAR_PLANE;

	glm_perspective(glm_rad(camera.fov), aspect, camera.near_plane, camera.far_plane, camera.projection);
	glm_mat4_identity(camera.view);

	camera.current_aspect = aspect;

	camera.properties = (AquaCameraProperties) { .show_properties_window = false };

	return camera;
}

void aqua_camera_update_projection(AquaCamera* camera) {
	glm_perspective(glm_rad(camera->fov), camera->current_aspect, camera->near_plane, camera->far_plane, camera->projection);
}

void aqua_camera_update_aspect(AquaCamera* camera, f32 aspect) {
	glm_perspective(glm_rad(camera->fov), aspect, camera->near_plane, camera->far_plane, camera->projection);
	camera->current_aspect = aspect;
}

void aqua_camera_update(AquaCamera* camera) {
 	vec3 center;
    glm_vec3_add(camera->position, camera->front, center);
    glm_lookat(camera->position, center, camera->up, camera->view);
}

void aqua_camera_imgui_update(AquaCamera* camera) {
	if (camera->properties.show_properties_window) {
		aqua_camera_imgui_properties_window(camera);
	}
}

void aqua_camera_imgui_properties_window(AquaCamera* camera) {
	igBegin("Camera Properties", &camera->properties.show_properties_window, ImGuiWindowFlags_None);

	igInputFloat3("Position", camera->position, "%0.2f", ImGuiInputTextFlags_None);
	if (igInputFloat("FOV", &camera->fov, 1.0f, 5.0f, "%0.2f", ImGuiInputTextFlags_None)) {
		aqua_camera_update_projection(camera);
	}
	if (igInputFloat("Near Plane", &camera->near_plane, 0.01f, 0.05f, "%0.2f", ImGuiInputTextFlags_None)) { aqua_camera_update_projection(camera); }
	if (igInputFloat("Far Plane", &camera->far_plane, 1.0f, 5.0f, "%0.2f", ImGuiInputTextFlags_None)) { aqua_camera_update_projection(camera); }
	igEnd();
}

void aqua_camera_destroy(AquaCamera* camera) {

}
