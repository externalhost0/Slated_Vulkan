
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

float3 pseudoNoise(float3 p) {
    return frac(sin(p * 13.37) * 43758.5453);
}

[shader("pixel")]
FSOutput fs_main(v2f input) {
    FSOutput output;

    float2 resolution = float2(1280.0, 720.0);
    float2 uv = input.UV * 2.0 - 1.0;
    uv.x *= resolution.x / resolution.y;

    float3 ro = float3(0.0, 0.0, perFrame.time);
    float3 rd = normalize(float3(uv, -2.25)); // focal length

    float3 color = float3(0.0, 0.0, 0.0);
    float t = 0.0;
    float time = perFrame.time;

    [loop]
    for (int i = 0; i < 60; ++i) {
        float3 p = t * rd + ro;

        // Safe rotation
        float T = (t + time) / 5.0;
        float c = cos(T), s = sin(T);
        float2 rot = mul(float2x2(c, -s, s, c), p.xy);
        p.x = rot.x;
        p.y = rot.y;

        // Slightly reduced wave detail
        for (float f = 0.0; f < 6.0; f += 1.0) { // down from 9 to 6
            float a = exp(f) / exp2(f); // original formula
            p += cos(p.yzx * a + time) / a;
        }

        // Distance function and contribution
        float d = 0.02 + abs((ro - p - float3(0.0, 1.0, 0.0)).y - 1.0) * 0.1;

        // Original colormap
        float PI = 3.14159265;
        float3 cm = pow(0.5 + 0.5 * cos(PI * t + float3(1, 2, 3)), float3(2.5, 2.5, 2.5));
        color += cm * 0.002 / d;

        t += d;

        if (t > 15.0) break;
    }

    // Tone mapping and gamma
    color *= color * color;
    color = 1.0 - exp(-color);
    color = pow(color, float3(1.0 / 2.2));

    output.FragColor = float4(color, 1.0);
    output.FragID = input.ID;
    return output;
}

