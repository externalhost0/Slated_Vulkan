#extension GL_EXT_buffer_reference: require

// descriptors
layout(set = 0, binding = 0) uniform SceneData {
    mat4 proj;
    mat4 view;
} sceneData;



struct Vertex {
    vec3 position;
    vec3 normal;
    vec2 uv;
};
layout(buffer_reference, std430) readonly buffer VertexBuffer {
    Vertex vertices[];
};

//push constants block
layout(push_constant) uniform Constants {
    mat4 model_matrix;
    uint id;
    VertexBuffer vertexBuffer;
} PushConstants;
