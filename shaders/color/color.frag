#version 410 core
out vec4 fragment_color;

in vec2 in_texture_coords;

uniform vec3 color;

void main() {
    fragment_color = vec4(color, 1.0);
}
