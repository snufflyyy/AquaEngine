#include "camera.h"
#include "cglm/cam.h"
#include "cglm/mat4.h"

AquaCamera aqua_camera_create(vec3 position, f32 fov, f32 aspect) {
	AquaCamera camera = {0};

	glm_vec3_copy(position, camera.position);

	glm_vec3_copy((vec3) { 0.0f, 0.0f, -1.0f }, camera.front);
	glm_vec3_copy((vec3) { 0.0f, 1.0f, 0.0f }, camera.up);

	camera.fov = fov;

	camera.near_plane = AQUA_CAMERA_DEFAULT_NEAR_PLANE;
	camera.far_plane = AQUA_CAMERA_DEFAULT_FAR_PLANE;

	glm_perspective(camera.fov, aspect, camera.near_plane, camera.far_plane, camera.projection);
	glm_mat4_identity(camera.view);

	return camera;
}

void aqua_camera_update(AquaCamera* camera) {
 	vec3 center;
    glm_vec3_add(camera->position, camera->front, center);
    glm_lookat(camera->position, center, camera->up, camera->view);
}

void aqua_camera_destroy(AquaCamera* camera) {

}
