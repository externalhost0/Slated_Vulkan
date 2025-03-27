// Standard Lit Shader
// ===========================
// === SHARED STRUCTS/DATA ===
// ===========================

import "BuiltIn.Common";
import "BuiltIn.Math";
import "BuiltIn.Lighting";
import "BuiltIn.Attribs";

// ===========================
// ====== VERTEX SHADER ======
// ===========================

struct v2f {
    // shader predefined
    float4 ClipPos : SV_Position;
    // user defined
    float3 Normal           : NORMAL;
    float2 UV               : TEXCOORD0;
    float3 WorldPosition    : POSITION;
    nointerpolation uint ID : ID;
};

[shader("vertex")]
v2f vs_main(VSInput input) {
    v2f output;

    Vertex v = pushConstants.vertexBufferAddress[input.VertexID];

    float3 worldpos = (pushConstants.model_matrix * float4(v.position, 1)).xyz;
    output.ClipPos = ((cameraData.proj * cameraData.view) * float4(worldpos, 1));

    output.Normal = (pushConstants.model_matrix * float4(v.normal, 0.0)).xyz;

    output.UV = float2(v.uv_x, v.uv_y);
    output.WorldPosition = worldpos;
    output.ID = pushConstants.id;

    return output;
}

// ===========================
// ===== FRAGMENT SHADER =====
// ===========================


[shader("pixel")]
FSOutput fs_main(v2f input) {
    FSOutput output;

    float3 lightResult = CalculateLighting(input.Normal, input.WorldPosition);
    output.FragColor = float4(lightResult, 1.0);
    output.FragID = input.ID;

    return output;
}
