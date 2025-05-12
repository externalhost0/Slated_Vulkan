// Global uniforms that are made available for EVERY shader
[require(spvShaderNonUniform)]

struct CameraData {
    float4x4 proj;
    float4x4 view;
    float3 position;
}

struct AmbientLight {
    float3 Color;
    float Intensity;
};
struct DirectionalLight {
    float3 Color;
    float Intensity;
    float3 Direction;
};
struct PointLight {
    float3 Color;
    float Intensity;
    float3 Position;
    float Range;
};
struct SpotLight {
    float3 Color;
    float Intensity;
    float3 Position;
    float Size;
    float3 Direction;
    float Blend;
}
static const uint8_t MAX_POINT_LIGHTS = 4;
static const uint8_t MAX_SPOT_LIGHTS = 4;

struct LightingData{
    AmbientLight _ambientLight;
    DirectionalLight _directionalLight;
    PointLight _pointLights[MAX_POINT_LIGHTS];
    SpotLight _spotLights[MAX_SPOT_LIGHTS];
}

struct PerFrameData {
    CameraData camera;
    LightingData lighting;
    float time;
}

// GLOBAL BUFFER DS
// i have no idea why it cant be in set 0
[[vk::binding(0, 3)]]
ConstantBuffer<PerFrameData> perFrame;
// TEXTURE DS
[[vk::binding(0, 0)]]
uniform Texture2D kTextures2D[];
// SAMPLER DS
[[vk::binding(1, 0)]]
uniform SamplerState kSamplers[];
// STORAGE DS
[[vk::binding(2, 0)]]
uniform __DynamicResource kStorage[];

// float4 textureBindless2D(uint textureid, uint samplerid, float2 uv) {
//     return kTextures2D[textureid].Sample(kSamplers[samplerid], uv);
// }