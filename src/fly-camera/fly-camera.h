#pragma once
#include <aqua/fly-camera.h>
#include "gfx/camera/camera.h"

struct AquaFlyCamera {
	AquaCamera* camera;
	AquaFlyCameraProperties properties;
	bool forward_pressed, backward_pressed, left_pressed, right_pressed;
};
