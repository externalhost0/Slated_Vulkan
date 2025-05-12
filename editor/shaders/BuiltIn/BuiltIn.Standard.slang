
// standard vertex struct for all meshes
struct Vertex {
    float3 position;
    float uv_x;
    float3 normal;
    float uv_y;
    float4 tangent;
};
// Standard Input into Vertex Shader
struct VSInput {
    uint VertexID : SV_VertexID;
};
struct FSOutput {
    float4 FragColor : SV_Target0;
    uint FragID : SV_Target1;
};

struct PerObjectData {
    float4x4 model;
    Ptr<Vertex> vertexBufferAddress;
    uint32_t id;
}
[[vk::push_constant]]
PerObjectData perObject;