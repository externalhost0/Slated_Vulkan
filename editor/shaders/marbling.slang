
import "BuiltIn.Standard";
import "BuiltIn.Common";
import "BuiltIn.Lighting";
import "BuiltIn.Math";

struct v2f {
    float4 ClipPos : SV_Position;
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

[shader("pixel")]
FSOutput fs_main(v2f input) {
    FSOutput output;

    float2 resolution = float2(1280.0, 720.0);
    float time = perFrame.time;

    // Convert input.UV to fragCoord space
    float2 fragCoord = input.UV * resolution;

    // Normalized and centered UV (range: ~[-1, 1])
    float2 uv = (2.0 * fragCoord - resolution) / min(resolution.x, resolution.y);

    // Iterative warping
    for (float i = 1.0; i < 10.0; i += 1.0) {
        uv.x += 0.6 / i * cos(i * 2.5 * uv.y + time);
        uv.y += 0.6 / i * cos(i * 1.5 * uv.x + time);
    }

    // Final color computation
    float glow = 0.1 / abs(sin(time - uv.y - uv.x));
    output.FragColor = float4(glow.xxx, 1.0);
    output.FragID = input.ID;
    return output;
}
