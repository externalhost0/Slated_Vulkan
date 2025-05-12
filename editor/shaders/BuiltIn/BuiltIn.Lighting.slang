// =========================
// === LIGHTING STRUCTS ====
// =========================
import "BuiltIn.Common";

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

float3 CalcSpotDiffuse(SpotLight lightObject, float3 normals, float3 direction, float distance_light) {

    // attenuation calculations
    float theta = dot(direction, normalize(-lightObject.Direction));
    float cosOuter = cos(radians(lightObject.Size * 0.5));
    float attenuation;
    if (lightObject.Blend == 0.0) {
        attenuation = step(cosOuter, theta);
    } else {
        float cosInner = cos(radians(lightObject.Size * 0.5 * (1.0 - lightObject.Blend)));
        attenuation = clamp((theta - cosOuter) / (cosInner - cosOuter), 0.0, 1.0);
    }

    float diff = max(dot(normals, direction), 0.0);
    float3 diffuse = diff * lightObject.Color;

    return attenuation * (lightObject.Intensity / distance_light) * diffuse;
}

float3 CalcSpotSpecular(SpotLight lightObject, float3 normals, float3 view_dir, float3 direction, float distance_light) {

    // attenuation calculations
    float theta = dot(direction, normalize(-lightObject.Direction));
    float cosOuter = cos(radians(lightObject.Size * 0.5));
    float attenuation;
    if (lightObject.Blend == 0.0) {
        attenuation = step(cosOuter, theta);
    } else {
        float cosInner = cos(radians(lightObject.Size * 0.5 * (1.0 - lightObject.Blend)));
        attenuation = clamp((theta - cosOuter) / (cosInner - cosOuter), 0.0, 1.0);
    }

    float3 halfwayDir = normalize(direction + view_dir);
    float spec = pow(max(dot(normals, halfwayDir), 0.0), 16.0);
    float3 specular = spec * lightObject.Color;

    return attenuation * (lightObject.Intensity / distance_light) * specular;
}

// general function that applies all lighting to a fragment
float3 CalculateLighting(float3 Normal, float3 WorldPosition) {

    float3 ambientResult = perFrame.lighting._ambientLight.Color * perFrame.lighting._ambientLight.Intensity;
    float3 diffuseResult = float3(0, 0, 0);
    float3 specularResult = float3(0, 0, 0);

    float3 view_direction = normalize(perFrame.camera.position - WorldPosition);
    float3 vertex_normals = normalize(Normal);

    diffuseResult += CalcDirectionalDiffuse(perFrame.lighting._directionalLight, vertex_normals, view_direction);
    specularResult += CalcDirectionalSpecular(perFrame.lighting._directionalLight, vertex_normals, view_direction);

    [unroll]
    for (int i = 0; i < MAX_POINT_LIGHTS; ++i) {
        float3 direction = normalize(perFrame.lighting._pointLights[i].Position - WorldPosition);
        float distancelight = length(perFrame.lighting._pointLights[i].Position - WorldPosition);

        diffuseResult += CalcPointDiffuse(perFrame.lighting._pointLights[i], vertex_normals, direction, distancelight);
        specularResult += CalcPointSpecular(perFrame.lighting._pointLights[i], vertex_normals, view_direction, direction, distancelight);
    }
    [unroll]
    for (int i = 0; i < MAX_SPOT_LIGHTS; ++i) {
        float3 direction = normalize(perFrame.lighting._spotLights[i].Position - WorldPosition);
        float distancelight = length(perFrame.lighting._spotLights[i].Position - WorldPosition);

        diffuseResult += CalcSpotDiffuse(perFrame.lighting._spotLights[i], vertex_normals, direction, distancelight);
        specularResult += CalcSpotSpecular(perFrame.lighting._spotLights[i], vertex_normals, view_direction, direction, distancelight);
    }

    return ambientResult + diffuseResult + specularResult;
}