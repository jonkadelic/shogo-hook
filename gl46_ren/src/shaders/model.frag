#version 460 core

in vec4 fs_color;
in vec3 fs_normal;
in vec2 fs_uv;

out vec4 out_color;

uniform sampler2D u_texture;

void main() {
    // Simple Lambertian shading with fixed light direction
    vec3 light_dir = normalize(vec3(0.5, 1.0, 0.5));
    float brightness = max(0.3, dot(normalize(fs_normal), light_dir));
    
    out_color = texture(u_texture, fs_uv) * vec4(fs_color.rgb, fs_color.a) * brightness;
}
