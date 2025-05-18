// global uniforms that are made available for EVERY shader
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
    float2 resolution;
}

// GLOBAL BUFFER SET
//set 3
[[vk::binding(0, 3)]]
ConstantBuffer<PerFrameData> perFrame;

// BINDLESS SETS
// sets 0-2
// TEXTURE DS
[[vk::binding(0, 0)]]
uniform Texture2D kTextures2D[];
[[vk::binding(0, 1)]]
uniform Texture3D kTextures3D[];
[[vk::binding(0, 2)]]
uniform TextureCube kTexturesCube[];
// SAMPLER DS
[[vk::binding(1, 0)]]
uniform SamplerState kSamplers[];
// STORAGE DS
[[vk::binding(2, 0)]]
uniform __DynamicResource kStorage[];


float4 textureBindless2D(uint textureid, uint samplerid, float2 uv) {
    return kTextures2D[NonUniformResourceIndex(textureid)].Sample(kSamplers[NonUniformResourceIndex(samplerid)], uv);
}
float4 textureBindless2DLod(uint textureid, uint samplerid, float2 uv, float lod) {
    return kTextures2D[NonUniformResourceIndex(textureid)].SampleLevel(kSamplers[NonUniformResourceIndex(samplerid)], uv, lod);
}
float4 textureBindlessCube(uint textureid, uint samplerid, float3 uvw) {
    return kTexturesCube[NonUniformResourceIndex(textureid)].Sample(kSamplers[NonUniformResourceIndex(samplerid)], uvw);
}
float4 textureBindlessCubeLod(uint textureid, uint samplerid, float3 uvw, float lod) {
    return kTexturesCube[NonUniformResourceIndex(textureid)].SampleLevel(kSamplers[NonUniformResourceIndex(samplerid)], uvw, lod);
}