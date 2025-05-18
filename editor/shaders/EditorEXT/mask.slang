
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
    float FragMask   : SV_Target0;
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
[shader("pixel")]
FSOutput fs_main(v2f input) {
    FSOutput output;
    output.FragMask = 1.0f;
    return output;
}