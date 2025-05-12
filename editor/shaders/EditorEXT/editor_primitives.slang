// Standard Editor Wireframe Shader
#define NOREFLECT

import "BuiltIn.Common";

struct Vertex {
    float3 position;
    float uv_x;
    float3 normal;
    float uv_y;
    float4 tangent;
};

// editor shaders implement their own push constants
struct PushConstants {
    float4x4 model_matrix;
    Ptr<Vertex> vertexBufferAddress;
}
[[vk::push_constant]]
PushConstants pushConstants;

// ===========================
// ====== VERTEX SHADER ======
// ===========================

struct VSInput {
    uint VertexID : SV_VertexID;
};
struct FSOutput {
    float4 FragColor : SV_Target0;
};
struct v2f {
    float4 ClipPos : SV_Position;
};

[shader("vertex")]
v2f vs_main(VSInput input) {
    v2f output;
    Vertex v = pushConstants.vertexBufferAddress[input.VertexID];
    
    float3 worldpos = mul(pushConstants.model_matrix, float4(v.position, 1.0)).xyz;
    output.ClipPos = mul(mul(perFrame.camera.proj, perFrame.camera.view), float4(worldpos, 1.0));
    return output;
}

// ===========================
// ===== FRAGMENT SHADER =====
// ===========================


[shader("pixel")]
FSOutput fs_main(v2f input) {
    FSOutput output;

    float fade_end = 50.f;
    float fade_start = 1.f;

    float depth = input.ClipPos.z / input.ClipPos.w;
    float fade = clamp((fade_end - depth) / (fade_end - fade_start), 0, 1);

    output.FragColor = float4(float3(0.5f), fade);
    return output;
}