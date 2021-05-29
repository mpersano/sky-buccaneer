#version 450 core

layout(points) in;
layout(triangle_strip, max_vertices=4) out;

uniform vec3 eyePosition;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

in WorldVertex {
    vec3 position;
    vec3 velocity;
    vec2 size;
} gs_in[];

void main(void)
{
    vec3 worldPosition = gs_in[0].position;
    vec2 size = gs_in[0].size;

    vec3 velVec = normalize(gs_in[0].velocity);
    vec3 eyeVec = eyePosition - worldPosition;
    vec3 eyeOnVelVecPlane = eyePosition - dot(eyeVec, velVec) * velVec;
    vec3 projectedEyeVec = eyeOnVelVecPlane - worldPosition;
    vec3 sideVec = normalize(cross(projectedEyeVec, velVec));

    vec3 spriteVerts[4];
    spriteVerts[0] = worldPosition - sideVec * 0.5 * size.x + velVec * 0.5 * size.y;
    spriteVerts[1] = spriteVerts[0] + sideVec * size.x;
    spriteVerts[2] = spriteVerts[0] + velVec * size.y;
    spriteVerts[3] = spriteVerts[1] + velVec * size.y;

    mat4 viewProjection = projectionMatrix * viewMatrix;

    gl_Position = viewProjection * vec4(spriteVerts[0], 1);
    EmitVertex();

    gl_Position = viewProjection * vec4(spriteVerts[1], 1);
    EmitVertex();

    gl_Position = viewProjection * vec4(spriteVerts[2], 1);
    EmitVertex();

    gl_Position = viewProjection * vec4(spriteVerts[3], 1);
    EmitVertex();
}
