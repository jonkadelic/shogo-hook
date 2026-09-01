#version 460 core

in vec2 fs_uv;

out vec4 out_color;

uniform sampler2D u_texture;

void main() {
    out_color = texture(u_texture, fs_uv);
    if (out_color.r == 0.0 && out_color.g == 0.0 && out_color.b == 0.0) {
        discard;
    }
}
