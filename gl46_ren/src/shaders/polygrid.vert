#version 460 core

struct VertexData {
    float position[3];
    float color[4];
    float uv[2];
};

layout(binding = 0, std430) readonly buffer ssbo0 {
    VertexData vertices[];
};
layout(binding = 1, std430) readonly buffer ssbo1 {
    int indices[];
};
layout(binding = 2, std430) readonly buffer ssbo2 {
    int offsets[];
};

vec3 get_position(int index) {
    return vec3(
        vertices[index].position[0],
        vertices[index].position[1],
        vertices[index].position[2]
    );
}

vec4 get_color(int index) {
    return vec4(
        vertices[index].color[0],
        vertices[index].color[1],
        vertices[index].color[2],
        vertices[index].color[3]
    );
}

vec2 get_uv(int index) {
    return vec2(
        vertices[index].uv[0],
        vertices[index].uv[1]
    );
}

int get_offset(int index) {
    // offsets are packed bytes, lowest index at LSB
    int offset = (offsets[index / 4] >> (8 * (index % 4))) & 0xFF;
    if (offset > 0x7F) {
        offset |= 0xFFFFFF00;
    }

    return offset;
}

int get_index(int index) {
    return indices[index];
}

out vec4 fs_color;
out vec2 fs_uv;

uniform mat4 u_projection;
uniform mat4 u_model;

void main() {
    int index = get_index(gl_VertexID);

    int offset = get_offset(index);
    vec3 pos = get_position(index);
    pos.y += offset;

    float br = (offset + 128.0) / 255.0;

    gl_Position = u_projection * u_model * vec4(pos, 1.0);
    fs_color = get_color(index) * vec4(br, br, br, 1.0);
    fs_uv = get_uv(index);
}
