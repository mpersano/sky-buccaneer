#version 420 core

layout(location=0) in vec3 position;
layout(location=1) in vec3 normal;
layout(location=2) in vec2 texcoord;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform mat4 lightViewProjection;
uniform mat3 normalMatrix;

out vec3 vs_position;
out vec3 vs_normal;
out vec4 vs_color;
out vec4 vs_positionInLightSpace;
out vec2 vs_texcoord;

void main(void)
{
    const mat4 shadowMatrix = mat4(0.5, 0.0, 0.0, 0.0,
                                   0.0, 0.5, 0.0, 0.0,
                                   0.0, 0.0, 0.5, 0.0,
                                   0.5, 0.5, 0.5, 1.0);

    vs_position = vec3(modelMatrix * vec4(position, 1.0));
    vs_positionInLightSpace = shadowMatrix * lightViewProjection * modelMatrix * vec4(position, 1.0);
    vs_normal = normalize(normalMatrix * normal);
    vs_color = vec4(1.0);
    vs_texcoord = texcoord;
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(position, 1.0);
}
