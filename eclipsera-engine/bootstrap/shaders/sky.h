#pragma once

inline const char* SKY_VS = R"(#version 330
in vec3 vertexPosition;
out vec3 dir;
uniform mat4 matView;
uniform mat4 matProjection;
void main() {
    dir = vertexPosition;
    mat3 R = mat3(matView);
    mat4 viewNoTrans = mat4(
        vec4(R[0], 0.0),
        vec4(R[1], 0.0),
        vec4(R[2], 0.0),
        vec4(0.0, 0.0, 0.0, 1.0)
    );
    gl_Position = matProjection * viewNoTrans * vec4(vertexPosition, 1.0);
})";

inline const char* SKY_FS = R"(#version 330
in vec3 dir;
out vec4 FragColor;
uniform vec3 innerColor;
uniform vec3 outerColor;
void main() {
    vec3 nd = normalize(dir);
    float t = clamp(nd.y * 0.5 + 0.5, 0.0, 1.0);
    vec3 col = mix(innerColor, outerColor, t);
    FragColor = vec4(pow(col, vec3(1.0/2.2)), 1.0);
})";
