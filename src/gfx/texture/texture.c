#include "texture.h"
#include <stdio.h>
#include <string.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include <cimgui.h>
#include <cimgui_impl.h>

#include "utils/base-types.h"

AquaTexture aqua_texture_create(const char* image_path) {
    i32 image_width, image_height, image_channels_count;
    stbi_uc* image = stbi_load(image_path, &image_width, &image_height, &image_channels_count, 4);
    if (!image) {
        fprintf(stderr, "[ERROR] [Texture] Failed to load image!\n");
        return (AquaTexture) { .failed = true };
    }

    AquaTexture texture;

    texture.width = (u32) image_width;
    texture.height = (u32) image_height;

    usize path_length = strlen(image_path);

    texture.image_path = (char*) malloc((path_length + 1) * sizeof(char));
    if (!texture.image_path) {
   		fprintf(stderr, "[ERROR] [Aqua] [Texture] Failed to allocate memory for image path!\n");
    	return (AquaTexture) { .failed = true };
    }

    strcpy(texture.image_path, image_path);

    glGenTextures(1, &texture.id);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture.id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(image);

    glBindTexture(GL_TEXTURE_2D, 0);

    texture.failed = false;

    return texture;
}

void aqua_texture_imgui_collapsible_header(AquaTexture* texture, const char* header_label) {
	if (igCollapsingHeader_BoolPtr(header_label, NULL, ImGuiTreeNodeFlags_None)) {
		igText("ID: %u", texture->id);
		igText("Image Path: %s", texture->image_path);
		igText("Width: %u", texture->width);
		igText("Height: %u", texture->height);

  		ImVec2 image_preview_size;
		if (texture->width > texture->height) {
			image_preview_size = (ImVec2) { (((f32) texture->width / texture->height) * 256.0f), 256.0f };
		} else {
			image_preview_size = (ImVec2) { 256.0f, (((f32) texture->height / texture->width) * 256.0f) };
		}
		igImage((ImTextureRef) { NULL, texture->id }, image_preview_size, (ImVec2) { 0.0f, 1.0f }, (ImVec2) { 1.0f, 0.0f });

		igSeparator();
	}
}

void aqua_texture_destroy(AquaTexture* texture) {
	if (texture->failed) { return; }

	if (texture->image_path) { free(texture->image_path); }
	glDeleteTextures(1, &texture->id);
}
