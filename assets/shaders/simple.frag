#version 450 core

uniform vec3 lightPosition;
uniform vec3 color;

in vec3 vs_position;
in vec3 vs_normal;

out vec4 fragColor;

void main(void)
{
    float intensity = max(dot(vs_normal, normalize(lightPosition - vs_position)), 0.0);
    fragColor = vec4(intensity * color, 1.0);
}
