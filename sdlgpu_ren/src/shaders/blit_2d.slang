struct LayoutUniforms {
    float2 position;
    float2 size;
};

[[vk::binding(0, 1)]]
ConstantBuffer<LayoutUniforms> u_layout;

struct VertexInput {
    float2 position;
    float2 uv;
};
struct VertexOutput {
    float4 position : SV_Position;
    float2 uv;
}

[shader("vertex")]
VertexOutput vertex_main(VertexInput input) {
    VertexOutput output;
    output.position = float4(
        input.position * u_layout.size + u_layout.position,
        0.0, 1.0
    );
    output.uv = input.uv;
    return output;
}

[[vk::binding(0, 2)]]
SamplerState u_sampler;
[[vk::binding(1, 2)]]
Texture2D u_texture;

[shader("fragment")]
float4 fragment_main(VertexOutput input) : SV_Target {
    return u_texture.Sample(u_sampler, input.uv);
}
