#version 460

#include "shared/scene_data.glsl"

layout(location = 0) out vec3 outColor;

void main() {
    Vertex v = PushConstants.vertexBuffer.vertices[gl_VertexIndex];

    gl_Position = (sceneData.proj * sceneData.view * PushConstants.model_matrix) * vec4(v.position, 1.0f);
    outColor = vec3(1.0f, 1.0f, 1.0f);
}
