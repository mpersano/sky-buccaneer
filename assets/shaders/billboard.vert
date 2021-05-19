#version 420 core

layout(location=0) in vec3 position;
layout(location=1) in vec3 velocity;

out WorldVertex {
    vec3 position;
    vec3 velocity;
} vs_out;

void main(void)
{
    vs_out.position = position;
    vs_out.velocity = velocity;
}
