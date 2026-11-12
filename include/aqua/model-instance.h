#pragma once
#include "types.h"

// Copyable placement. Detach before unloading the borrowed model handle.
typedef struct AquaModelInstance {
	AquaModelHandle model;
	mat4 transform;
} AquaModelInstance;

AquaModelInstance aqua_model_instance_create(AquaModelHandle model);
// XYZ Euler angles in degrees; replaces the matrix with translation * rotation * scale.
void aqua_model_instance_set_transform(AquaModelInstance* instance, vec3 position, vec3 rotation_degrees, vec3 scale);
