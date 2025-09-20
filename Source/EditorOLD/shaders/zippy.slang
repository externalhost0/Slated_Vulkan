
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

    float2 u = input.UV;
    float2 v = (1280, 720) / 200;
    u = 0.2 * (u + u - v) / v.y;

    float4 o = float4(1, 2, 3, 0);
    float4 z = o;

    float a = 0.5;
    float t = perFrame.time;

    for (float i = 0.0; ++i < 19.0; )
    {
        float len_term = length((1.0 + i * dot(v, v)) * sin(1.5 * u / (0.5 - dot(u, u)) - 9.0 * u.yx + t));
        o += (1.0 + cos(z + t)) / len_term;

        v = cos(++t - 7.0 * u * pow(a += 0.03, i)) - 5.0 * u;

        float2x2 m = float2x2(cos(i + 0.02 * t), cos(i + 0.02 * t + 11.0),
                              cos(i + 0.02 * t + 33.0), cos(i + 0.02 * t));

        u = mul(m, u);

        u += tanh(40.0 * dot(u, u) * cos(100.0 * u.yx + t)) / 200.0
           + 0.2 * a * u
           + cos(4.0 / exp(dot(o, o) / 100.0) + t) / 300.0;
    }

    o = 25.6 / (min(o, 13.0) + 164.0 / o) - dot(u, u) / 250.0;


    output.FragColor = float4(o.xyz, 1.0);
    output.FragID = input.ID;

    return output;
}