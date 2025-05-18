// Standard Lit Shader

// defines for uniforms can not be imported, only pure logic can be imported
// we can get around this with specific slang compilation api commands
import "BuiltIn.Standard";
import "BuiltIn.Common";
import "BuiltIn.Lighting";
import "BuiltIn.Math";
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

    Vertex v = perObject.vertexBufferAddress[input.VertexID];

    float3 worldpos = (perObject.model * float4(v.position, 1)).xyz;
    output.ClipPos = ((perFrame.camera.proj * perFrame.camera.view) * float4(worldpos, 1));

    output.Normal = (perObject.model * float4(v.normal, 0.0)).xyz;
    output.UV = float2(v.uv_x, v.uv_y);
    output.WorldPosition = worldpos;
    output.ID = perObject.id;

    return output;
}

// ===========================
// ===== FRAGMENT SHADER =====
// ===========================

// uniform float3 albedo;
// uniform float metallic;
// uniform float roughness;
// uniform float ao;

[shader("pixel")]
FSOutput fs_main(v2f input) {
    FSOutput output;

    float3 lightResult = CalculateLighting(input.Normal, input.WorldPosition);

    float3 n = normalize(input.Normal);
    float3 v = normalize(perFrame.camera.position - input.WorldPosition);
    float3 r = reflect(-v, n);
    float4 colorRefl = textureBindlessCube(14, 0, r);
    float4 Ka = colorRefl * 0.3f;

    output.FragColor = float4(lightResult + Ka.xyz, 1.0);
    output.FragID = input.ID;

    return output;
}
