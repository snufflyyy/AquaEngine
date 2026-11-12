#include <aqua/model-instance.h>

AquaModelInstance aqua_model_instance_create(AquaModelHandle model) {
	AquaModelInstance instance = {
		.model = model,
		.transform = GLM_MAT4_IDENTITY_INIT,
	};

	return instance;
}

void aqua_model_instance_set_transform(AquaModelInstance* instance, vec3 position, vec3 rotation_degrees, vec3 scale) {
	if (!instance) { return; }

	vec3 rotation_radians = {
		glm_rad(rotation_degrees[0]),
		glm_rad(rotation_degrees[1]),
		glm_rad(rotation_degrees[2]),
	};

	mat4 rotation;
	glm_euler_xyz(rotation_radians, rotation);

	glm_translate_make(instance->transform, position);
	glm_mat4_mul(instance->transform, rotation, instance->transform);
	glm_scale(instance->transform, scale);
}
