// Standard Editor Wireframe Shader
struct CameraData {
    float4x4 proj;
    float4x4 view;
};
[[vk::binding(0, 0)]]
ConstantBuffer<CameraData> cameraData;

struct Vertex {
    float3 position;
    float uv_x;
    float3 normal;
    float uv_y;
};
struct PushConstants {
    float4x4 model_matrix;
    uint id;
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
    float2 UV : TEXCOORD0;
    nointerpolation uint ID : ID;
};

[shader("vertex")]
v2f vs_main(VSInput input) {
    v2f output;
    Vertex v = pushConstants.vertexBufferAddress[input.VertexID];

    float3 worldpos = mul(pushConstants.model_matrix, float4(v.position, 1.0)).xyz;
    output.ClipPos = mul(mul(cameraData.proj, cameraData.view), float4(worldpos, 1.0));

    output.UV = float2(v.uv_x, v.uv_y);
    output.ID = pushConstants.id;
    return output;
}

// ===========================
// ===== FRAGMENT SHADER =====
// ===========================

struct FSOutput {
    float4 FragColor : SV_Target0;
    uint FragID      : SV_Target1;
};

[[vk::binding(1, 0)]]
Sampler2D u_Texture;

struct ExtraUBO {
    float4 color;
}
[[vk::binding(2, 0)]]
ConstantBuffer<ExtraUBO> extra;

[shader("pixel")]
FSOutput fs_main(v2f input) {
    FSOutput output;

    float threshold = 0.5f;
    float sampled = u_Texture.Sample(input.UV).r;
    sampled = clamp(sampled, 0, 1);
    float alpha = 1.0 - sampled;

    output.FragColor = float4(extra.color.xyz, alpha);
    output.FragID = input.ID;
    return output;
}
