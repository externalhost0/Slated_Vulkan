#version 460


layout(location = 0) in vec3 inColor;
layout(location = 0) out vec4 fragColor;

void main() {
    float u_FadeEnd = 50.0f;
    float u_FadeStart = 1.0f;
    // linearized depth value
    float depth = gl_FragCoord.z / gl_FragCoord.w;
    // fade factor
    float fade = clamp((u_FadeEnd - depth) / (u_FadeEnd - u_FadeStart), 0.0, 1.0);

    fragColor = vec4(inColor * fade, 1.0);
}
