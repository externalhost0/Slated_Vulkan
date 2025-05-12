// Standard Editor Image Shader
#define NOREFLECT

import "BuiltIn.Common";

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
    Ptr<Vertex> vertexBufferAddress;
    uint32_t id;
    uint textureId;
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
    float2 UV : TEXCOORD0;
    nointerpolation uint ID : ID;
};

[shader("vertex")]
v2f vs_main(VSInput input) {
    v2f output;
    Vertex v = pushConstants.vertexBufferAddress[input.VertexID];

    float3 worldpos = mul(pushConstants.model_matrix, float4(v.position, 1.0)).xyz;
    output.ClipPos = mul(mul(perFrame.camera.proj, perFrame.camera.view), float4(worldpos, 1.0));

    output.UV = float2(v.uv_x, v.uv_y);
    output.ID = pushConstants.id;
    return output;
}

// ===========================
// ===== FRAGMENT SHADER =====
// ===========================

[shader("pixel")]
FSOutput fs_main(v2f input) {
    FSOutput output;

    float threshold = 0.5f;
    
    float4 c = kTextures2D[NonUniformResourceIndex(pushConstants.textureId)].Sample(kSamplers[NonUniformResourceIndex(0)], input.UV);
    float sampled = c.r;
    sampled = clamp(sampled, 0, 1);
    float alpha = 1.0 - sampled;

    output.FragColor = float4(pushConstants.color, alpha);
    output.FragID = input.ID;
    return output;
}