import "BuiltIn.Common";

// parameters
static constexpr float GRID_SIZE = 50.f;
static constexpr float GRID_CELL_SIZE = 0.5f;
static constexpr float GRID_OPACITY = 0.5f;
static constexpr float GRID_MIN_PIXEL_BETWEEN_CELL = 10.0f;
// colors
static constexpr float4 GRID_COLOR_THIN = float4(0.5, 0.5, 0.5, GRID_OPACITY);
static constexpr float4 GRID_COLOR_THICK = float4(0.5, 0.5, 0.5, GRID_OPACITY+0.3);


struct v2f {
    float4 ClipPos : SV_Position;
    float2 CamPos  : CAM_POSITION;
    float2 UV      : TEXCOORD0;
};

[shader("vertex")]
v2f vs_main(uint VertexID : SV_VertexID) {
    v2f output;

    const float3 pos[4] = float3[4](
        float3(-1.0, 0.0, -1.0),
        float3( 1.0, 0.0, -1.0),
        float3( 1.0, 0.0,  1.0),
        float3(-1.0, 0.0,  1.0)
    );
    const int indices[6] = int[6](
        0, 1, 2, 2, 3, 0
    );

    int idx = indices[VertexID];
    float3 position = pos[idx] * GRID_SIZE;
    position.x += perFrame.camera.position.x;
    position.z += perFrame.camera.position.z;
  
    float4 origin = float4(0);
    position += origin.xyz;
  
    output.CamPos = perFrame.camera.position.xz;

    float3 worldpos = float4(position, 0).xyz;
    output.ClipPos = mul(mul(perFrame.camera.proj, perFrame.camera.view), float4(worldpos, 1));

    output.UV = position.xz;
    return output;
}


float log10f(float x) {
    return log(x) / log(10.0);
}
float satf(float x) {
    return saturate(x);
}
float2 satv(float2 x) {
    return clamp(x, float2(0.0, 0.0), float2(1.0, 1.0));
}
float max2(float2 v) {
    return max(v.x, v.y);
}
float mod_glsl(float x, float y) {
    return x - y * floor(x / y);
}

float2 mod_glsl(float2 x, float2 y) {
    return x - y * floor(x / y);
}

float4 gridColor(float2 uv, float2 camPos) {
    float2 dudv = float2(
        length(float2(ddx(uv.x), ddy(uv.x))),
        length(float2(ddx(uv.y), ddy(uv.y)))
    );

    float lodLevel = max(0.0, log10f((length(dudv) * GRID_MIN_PIXEL_BETWEEN_CELL) / GRID_CELL_SIZE) + 1.0);
    float lodFade = frac(lodLevel);

    float lod0 = GRID_CELL_SIZE * pow(10.0, floor(lodLevel));
    float lod1 = lod0 * 10.0;
    float lod2 = lod1 * 10.0;

    dudv *= 4.0;
    uv += dudv * 0.5;

    float lod0a = max2(1.0 - abs(satv(mod_glsl(uv, lod0) / dudv * 2.0 - 1.0)));
    float lod1a = max2(1.0 - abs(satv(mod_glsl(uv, lod1) / dudv * 2.0 - 1.0)));
    float lod2a = max2(1.0 - abs(satv(mod_glsl(uv, lod2) / dudv * 2.0 - 1.0)));
    
    uv -= camPos;

    float4 c;
    if (lod2a > 0.0) {
        c = GRID_COLOR_THICK;
    } else if (lod1a > 0.0) {
        c = lerp(GRID_COLOR_THICK, GRID_COLOR_THIN, lodFade); // mix → lerp
    } else {
        c = GRID_COLOR_THIN;
    }

    float opacityFalloff = 1.0 - satf(length(uv) / GRID_SIZE);
    c.a *= ((lod2a > 0.0) ? lod2a : ((lod1a > 0.0) ? lod1a : (lod0a * (1.0 - lodFade)))) * opacityFalloff;
    return c;
}

struct FSOutput {
    float4 FragColor : SV_Target0;
};
[shader("pixel")]
FSOutput fs_main(v2f input) {
    FSOutput output;
    output.FragColor = gridColor(input.UV, input.CamPos);
    return output;
}