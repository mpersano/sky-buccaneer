#version 420 core

layout(location=0) in vec3 position;
layout(location=1) in vec3 velocity;
layout(location=2) in vec2 size;

out WorldVertex {
    vec3 position;
    vec3 velocity;
    vec2 size;
} vs_out;

void main(void)
{
    vs_out.position = position;
    vs_out.velocity = velocity;
    vs_out.size = size;
}
