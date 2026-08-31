#version 460 core

in vec4 fs_color;
in vec2 fs_uv;

out vec4 out_color;

uniform sampler2D u_texture;

void main() {
    out_color = texture(u_texture, fs_uv) * vec4(fs_color.rgb, fs_color.a);
    // out_color = vec4(fs_uv.x, fs_uv.y, fs_uv.x * fs_uv.y, 1.0) * fs_color;
}
