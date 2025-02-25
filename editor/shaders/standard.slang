// Standard Lit Shader
// ===========================
// === SHARED STRUCTS/DATA ===
// ===========================

struct CameraData {
    float4x4 proj;
    float4x4 view;
    float3 position;
};
[[vk::binding(0, 0)]]
ConstantBuffer<CameraData> cameraData;

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
static const uint8_t MAX_POINT_LIGHTS = 4;

struct LightingUBO {
    AmbientLight _ambientLight;
    DirectionalLight _directionalLight;
    PointLight _pointLights[MAX_POINT_LIGHTS];
}
[[vk::binding(1, 0)]]
ConstantBuffer<LightingUBO> lighting;


struct Vertex {
    float3 position;
    float uv_x;
    float3 normal;
    float uv_y;
};
struct PushConstants {
    float4x4 model_matrix;
    uint id;
    Ptr<Vertex> vertexBufferAddress;
}
struct PerModel {
    float4x4 model_matrix;
    float4x4 inverse_transpose_model_matrix;
}
[[vk::push_constant]]
ConstantBuffer<PushConstants> pushConstants;


// ===========================
// ====== VERTEX SHADER ======
// ===========================

struct VSInput {
    uint VertexID : SV_VertexID;
};

struct v2f {
    float4 ClipPos : SV_Position;
    float3 Normal           : NORMAL;
    float2 UV               : TEXCOORD0;
    float3 WorldPosition    : POSITION;
    nointerpolation uint ID : ID;
};

float4   operator*(float4x4 matrix, float4 vec) { return mul(matrix, vec); }
float4   operator*(float4 vec, float4x4 matrix) { return mul(vec, matrix); }
float4x4 operator*(float4x4 matrixLeft, float4x4 matrixRight) { return mul(matrixLeft, matrixRight); }

float3   operator*(float3x3 matrix, float3 vec) { return mul(matrix, vec); }
float3   operator*(float3 vec, float3x3 matrix) { return mul(vec, matrix); }
float3x3 operator*(float3x3 matrixLeft, float3x3 matrixRight) { return mul(matrixLeft, matrixRight); }

[shader("vertex")]
v2f vs_main(VSInput input) {
    v2f output;

    Vertex v = pushConstants.vertexBufferAddress[input.VertexID];

    // float3 worldpos = mul(pushConstants.model_matrix, float4(v.position, 1.0)).xyz;
    // output.ClipPos = mul(mul(cameraData.proj, cameraData.view), float4(worldpos, 1.0));

    float3 worldpos = (pushConstants.model_matrix * float4(v.position, 1)).xyz;
    output.ClipPos = ((cameraData.proj * cameraData.view) * float4(worldpos, 1));

    // output.Normal = mul(-transpose(pushConstants.model_matrix), float4(v.normal, 0.0)).xyz;
    output.Normal = (pushConstants.model_matrix * float4(v.normal, 0.0)).xyz;

    output.UV = float2(v.uv_x, v.uv_y);
    output.WorldPosition = worldpos;
    output.ID = pushConstants.id;

    return output;
}

// ===========================
// ===== FRAGMENT SHADER =====
// ===========================

struct FSOutput {
    float4 FragColor : SV_Target0;
    uint FragID      : SV_Target1;
};


struct Material {
    float3 color;
}
// function forward declarations
float3 CalcDirectionalDiffuse(DirectionalLight lightObject, float3 normals, float3 view_dir);
float3 CalcDirectionalSpecular(DirectionalLight lightObject, float3 normals, float3 view_dir);
float3 CalcPointDiffuse(PointLight lightObject, float3 normals, float3 direction, float distancelight);
float3 CalcPointSpecular(PointLight lightObject, float3 normals, float3 view_dir, float3 direction, float distancelight);

[shader("pixel")]
FSOutput fs_main(v2f input) {
    FSOutput output;

    float3 ambientResult = lighting._ambientLight.Color * lighting._ambientLight.Intensity;
    float3 diffuseResult = float3(0, 0, 0);
    float3 specularResult = float3(0, 0, 0);

    float3 view_direction = normalize(cameraData.position - input.WorldPosition);
    float3 vertex_normals = normalize(input.Normal);

    diffuseResult += CalcDirectionalDiffuse(lighting._directionalLight, vertex_normals, view_direction);
    specularResult += CalcDirectionalSpecular(lighting._directionalLight, vertex_normals, view_direction);

    [unroll]
    for (int i = 0; i < MAX_POINT_LIGHTS; ++i) {
        float3 direction = normalize(lighting._pointLights[i].Position - input.WorldPosition);
        float distancelight = length(lighting._pointLights[i].Position - input.WorldPosition);

        diffuseResult += CalcPointDiffuse(lighting._pointLights[i], vertex_normals, direction, distancelight);
        specularResult += CalcPointSpecular(lighting._pointLights[i], vertex_normals, view_direction, direction, distancelight);
    }

    float3 lightResult = ambientResult + diffuseResult + specularResult;
    output.FragColor = float4(lightResult, 1.0);
    output.FragID = input.ID;

    return output;
}

// ===========================
// === LIGHTING FUNCTIONS ====
// ===========================

float3 CalcDirectionalDiffuse(DirectionalLight lightObject, float3 normals, float3 view_dir) {
    float3 lightDir = normalize(-lightObject.Direction);
    float diff = max(dot(normals, lightDir), 0.0);
    float intensity = lightObject.Intensity / 10.0;
    return intensity * lightObject.Color * diff;
}

float3 CalcDirectionalSpecular(DirectionalLight lightObject, float3 normals, float3 view_dir) {
    float3 lightDir = normalize(-lightObject.Direction);
    float3 halfwayDir = normalize(lightDir + view_dir);
    float spec = pow(max(dot(normals, halfwayDir), 0.0), 16.0);
    float intensity = lightObject.Intensity / 10.0;
    return intensity * lightObject.Color * spec;
}

float3 CalcPointDiffuse(PointLight lightObject, float3 normals, float3 direction, float distance_light) {
    float r = lightObject.Range;
    float attenuation = clamp(1.0 - (distance_light * distance_light) / (r * r), 0.0, 1.0);
    float diff = max(dot(normals, direction), 0.0);
    float3 diffuse = diff * lightObject.Color;
    return attenuation * (lightObject.Intensity / distance_light) * diffuse;
}

float3 CalcPointSpecular(PointLight lightObject, float3 normals, float3 view_dir, float3 direction, float distance_light) {
    float r = lightObject.Range;
    float attenuation = clamp(1.0 - (distance_light * distance_light) / (r * r), 0.0, 1.0);
    float3 halfwayDir = normalize(direction + view_dir);
    float spec = pow(max(dot(normals, halfwayDir), 0.0), 16.0);
    float3 specular = spec * lightObject.Color;
    return attenuation * (lightObject.Intensity / distance_light) * specular;
}