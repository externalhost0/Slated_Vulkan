// Standard Editor Wireframe Shader
#define NOREFLECT

struct CameraData {
    float4x4 proj;
    float4x4 view;
    float3 position;
};
[[vk::binding(0, 0)]]
ConstantBuffer<CameraData> cameraData;

struct Vertex {
    float3 position;
    float uv_x;
    float3 normal;
    float uv_y;
    float4 tangent;
};
struct PushConstants {
    float4x4 model_matrix;
    float3 color;
    uint32_t id;
    Ptr<Vertex> vertexBufferAddress;
}
[[vk::push_constant]]
ConstantBuffer<PushConstants> pushConstants;

// ===========================
// ====== VERTEX SHADER ======
// ===========================

struct VSInput {
    uint VertexID : SV_VertexID;
};

struct v2f {
    float4 ClipPos : SV_Position;
    float3 Color : COLOR;
};

[shader("vertex")]
v2f vs_main(VSInput input) {
    v2f output;
    Vertex v = pushConstants.vertexBufferAddress[input.VertexID];

    float3 worldpos = mul(pushConstants.model_matrix, float4(v.position, 1.0)).xyz;
    output.ClipPos = mul(mul(cameraData.proj, cameraData.view), float4(worldpos, 1.0));
    output.Color = pushConstants.color;
    return output;
}

// ===========================
// ===== FRAGMENT SHADER =====
// ===========================

struct FSOutput {
    float4 FragColor : SV_Target0;
};

[shader("pixel")]
FSOutput fs_main(v2f input) {
    FSOutput output;

    float fade_end = 50.f;
    float fade_start = 1.f;

    float depth = input.ClipPos.z / input.ClipPos.w;
    float fade = clamp((fade_end - depth) / (fade_end - fade_start), 0, 1);

    output.FragColor = float4(input.Color, fade);
    return output;
}