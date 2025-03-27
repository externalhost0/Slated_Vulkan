// this file is included in every shader file
// and therefore does not need to be imported

// Standard Input into Vertex Shader
struct VSInput {
    uint VertexID : SV_VertexID;
};
struct FSOutput {
    float4 FragColor : SV_Target0;
    uint FragID : SV_Target1;
};
// Global Uniforms that are made available for EVERY shader

struct CameraData {
    float4x4 proj;
    float4x4 view;
    float3 position;
};
[[vk::binding(0, 0)]]
ConstantBuffer<CameraData> cameraData;

struct PushConstants {
    float4x4 model_matrix;
    Ptr<Vertex> vertexBufferAddress;
    uint32_t id;
}
[[vk::push_constant]]
ConstantBuffer<PushConstants> pushConstants;

// Standard Vertex struct for all meshes
struct Vertex {
    float3 position;
    float uv_x;
    float3 normal;
    float uv_y;
    float4 tangent;
};

// Helper Functions
// simplifies model transformation for 3D
float4 ToClipPos(float3 position) {
    float3 world_pos = mul(pushConstants.model_matrix, float4(position, 1)).xyz;
    return mul(mul(cameraData.proj, cameraData.view), float4(world_pos, 1));
}