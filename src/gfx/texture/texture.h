#pragma once

#include <stdbool.h>

#include <glad/glad.h>

#include "utils/base-types.h"

#define AQUA_TEXTURE_HANDLE_INVALID USIZE_MAX

typedef struct AquaTexture {
	bool failed;

	GLuint id;

	u32 width;
	u32 height;

	char* image_path;
} AquaTexture;

typedef usize AquaTextureHandle;

AquaTexture aqua_texture_create(const char* image_path);
AquaTexture aqua_texture_create_from_memory(const u8* buffer, u32 buffer_length);
void aqua_texture_imgui_collapsible_header(AquaTexture* texture, const char* header_label);
void aqua_texture_destroy(AquaTexture* texture);
