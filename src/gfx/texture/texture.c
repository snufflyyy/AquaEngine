#include "texture.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include <cimgui.h>
#include <cimgui_impl.h>

#include <aqua/types.h>

AquaTexture aqua_texture_create_from_pixels(const u8* rgba_pixels, u32 width, u32 height) {
    if (!rgba_pixels || width == 0 || height == 0) {
        fprintf(stderr, "[ERROR] [Aqua] [Texture] Invalid pixel data (%ux%u)!\n", width, height);
        return (AquaTexture) { .failed = true };
    }

    AquaTexture texture = {
        .failed = false,
        .width = width,
        .height = height,
        .image_path = NULL,
    };

    glGenTextures(1, &texture.id);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture.id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei) width, (GLsizei) height, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba_pixels);
    glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);

    return texture;
}

AquaTexture aqua_texture_create(const char* image_path) {
    i32 image_width, image_height, image_channels_count;
    stbi_uc* image = stbi_load(image_path, &image_width, &image_height, &image_channels_count, 4);
    if (!image) {
        fprintf(stderr, "[ERROR] [Aqua] [Texture] Failed to load image '%s': %s\n", image_path, stbi_failure_reason());
        return (AquaTexture) { .failed = true };
    }

    AquaTexture texture = aqua_texture_create_from_pixels(image, (u32) image_width, (u32) image_height);
    stbi_image_free(image);
    if (texture.failed) { return texture; }

    usize path_length = strlen(image_path);
    texture.image_path = (char*) malloc(path_length + 1);
    if (!texture.image_path) {
        fprintf(stderr, "[ERROR] [Aqua] [Texture] Failed to allocate memory for image path!\n");
        glDeleteTextures(1, &texture.id);
        return (AquaTexture) { .failed = true };
    }
    memcpy(texture.image_path, image_path, path_length + 1);

    return texture;
}

AquaTexture aqua_texture_create_from_memory(const u8* buffer, u32 buffer_length) {
    i32 image_width, image_height, image_channels_count;
    stbi_uc* image = stbi_load_from_memory(buffer, (i32) buffer_length, &image_width, &image_height, &image_channels_count, 4);
    if (!image) {
        fprintf(stderr, "[ERROR] [Aqua] [Texture] Failed to load image from memory: %s\n", stbi_failure_reason());
        return (AquaTexture) { .failed = true };
    }

    AquaTexture texture = aqua_texture_create_from_pixels(image, (u32) image_width, (u32) image_height);
    stbi_image_free(image);

    return texture;
}

void aqua_texture_imgui_collapsible_header(const AquaTexture* texture, const char* header_label) {
	if (igCollapsingHeader_BoolPtr(header_label, NULL, ImGuiTreeNodeFlags_None)) {
		igText("ID: %u", texture->id);
		igText("Image Path: %s", texture->image_path);
		igText("Width: %u", texture->width);
		igText("Height: %u", texture->height);

  		ImVec2 image_preview_size;
		if (texture->width > texture->height) {
			image_preview_size = (ImVec2) { (((f32) texture->width / (f32) texture->height) * 256.0f), 256.0f };
		} else {
			image_preview_size = (ImVec2) { 256.0f, (((f32) texture->height / (f32) texture->width) * 256.0f) };
		}
		igImage((ImTextureRef) { NULL, texture->id }, image_preview_size, (ImVec2) { 0.0f, 0.0f }, (ImVec2) { 1.0f, 1.0f });

		igSeparator();
	}
}

void aqua_texture_destroy(AquaTexture* texture) {
	if (texture->failed) { return; }

	if (texture->image_path) { free(texture->image_path); }
	glDeleteTextures(1, &texture->id);
}
