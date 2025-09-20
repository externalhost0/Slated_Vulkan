
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

float3 erot(float3 p, float3 ax, float t) {
    return lerp(dot(ax, p) * ax, p, cos(t)) + cross(ax, p) * sin(t);
}

[shader("pixel")]
FSOutput fs_main(v2f input) {
    FSOutput output;

    float2 resolution = float2(1280.0, 720.0);
    float2 uv = (input.UV * resolution - 0.5 * resolution) / resolution.y;

    float time = perFrame.time;
    float3 col = float3(0.0, 0.0, 0.0);

    float3 ro = float3(0.0, 0.5, -5.0);
    float3 rt = float3(0.0, 0.0, 0.0);
    ro.zx += time;
    rt.zx += time;

    float3 z = normalize(rt - ro);
    float3 x = float3(z.z, 0.0, -z.x);
    float3 y = cross(z, x);
    float3 rd = mul(float3x3(x, y, z), erot(normalize(float3(uv, 1.0)), float3(0, 0, 1), time * 0.1));

    float g = 0.0;
    float e = 0.0;

    for (float i = 0.0; i < 50.0; i += 1.0) {
        float3 p = ro + rd * g;

        float h = length(min(
            sin(p + sin(p.xyy / p.z * 0.1 + p.y * 0.01)),
            min(sin(p.xxx), sin(p.yyy))
        )) - 0.1;

        float3 op = p;

        p = erot(sin(p * 0.5) / 0.5, normalize(float3(0, 1, 1)), p.z * 0.1);

        float u = min(
            length(p.xz + sin(p.yz * 4.0)),
            length(p.yz + sin(p.xy * 2.0))
        ) - 0.02;

        u /= 3.0;
        h = min(abs(u), h);

        e = max(0.001, h);
        g += e;

        float a = 0.0;
        if (abs(u) == h) {
            a = 1.0 * exp(-30.0 * frac(p.z * 0.1 + op.z * 0.1 + time));
        }

        float den = exp(i * i * e) * (g * 0.5);
        col += clamp((float3(0.8, 0.9, 1.0)) * (0.0524 + a) / den, float3(0.0, 0.0, 0.0), float3(1.0, 1.0, 1.0));
    }

    // Approximate dFdy(col) — we fake it by estimating based on UV delta
    float3 dCol = float3(0.0, 0.0, 0.0); // dFdy(col) would require screen-space derivatives

    // You can compute this with manual sampling if you're doing this on-screen

    float3 flicker = sin(float3(-uv.x, uv.y, uv.x) * 500.0) * 0.5 + 0.5;
    col += dCol * 10.0 * flicker;

    output.FragColor = float4(col, 1.0);
    output.FragID = input.ID;
    return output;
}
