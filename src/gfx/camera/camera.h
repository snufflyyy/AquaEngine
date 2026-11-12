#pragma once
#include <aqua/camera.h>
#include <aqua/debug.h>

struct AquaCamera {
	AquaCameraProperties properties;
	mat4 view, projection;
};

void aqua_camera_imgui_properties_window(AquaCamera* camera);
