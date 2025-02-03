#version 460
#extension GL_EXT_nonuniform_qualifier: require

#include "shared/scene_data.glsl"

layout(location = 0) in vec3 inNormal;
layout(location = 1) in vec2 inUV;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out uint fragID;

void main() {
    fragColor = vec4(inUV, 0.0, 1.0);
    fragID = PushConstants.id;
}
