#version 450 core

layout(location=0) in vec3 position;
layout(location=1) in vec3 normal;

out vec3 vs_position;
out vec3 vs_normal;

uniform mat4 mvp;
uniform mat4 modelMatrix;
uniform mat3 normalMatrix;

void main(void)
{
    vs_position = vec3(modelMatrix * vec4(position, 1.0));
    vs_normal = normalize(normalMatrix * normal);
    gl_Position = mvp * vec4(position, 1.0);
}
