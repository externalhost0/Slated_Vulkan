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
    float3 color;
    uint32_t id;
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
    uint FragID      : SV_Target1;
};
struct v2f {
    float4 ClipPos : SV_Position;
    float3 Normal  : NORMAL;
    float3 WorldPosition    : POSITION;
};

[shader("vertex")]
v2f vs_main(VSInput input) {
    v2f output;
    Vertex v = pushConstants.vertexBufferAddress[input.VertexID];
    
    float3 worldpos = mul(pushConstants.model_matrix, float4(v.position, 1.0)).xyz;
    output.ClipPos = mul(mul(perFrame.camera.proj, perFrame.camera.view), float4(worldpos, 1.0));
    output.Normal = normalize(mul(pushConstants.model_matrix, float4(v.normal, 0.0)).xyz);
    output.WorldPosition = worldpos;

    return output;
}

// ===========================
// ===== FRAGMENT SHADER =====
// ===========================


[shader("pixel")]
FSOutput fs_main(v2f input) {
    FSOutput output;

    float3 viewDir = normalize(perFrame.camera.position - input.WorldPosition);
    float NdotV = max(dot(input.Normal, viewDir), 0.0f);
    float smoothedNdotV = pow(NdotV, 0.5);

    output.FragColor = float4(pushConstants.color * smoothedNdotV, 1);
    output.FragID = pushConstants.id;
    return output;
}