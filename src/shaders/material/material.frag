#version 410 core
out vec4 fragment_color;

in vec2 in_texture_coords;

uniform sampler2D in_texture;

uniform vec4 color;
uniform float color_strength;

void main() {
	vec4 texture_color = texture(in_texture, in_texture_coords);
    if (texture_color.a <= 0.0) {
        discard;
    }

    fragment_color = mix(texture_color, color, color_strength);
}
