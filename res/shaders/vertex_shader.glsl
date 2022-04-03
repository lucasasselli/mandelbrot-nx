#version 330 core

layout (location = 0) in vec3 vPos;

uniform vec2 offset;
uniform float zoom;

out vec2 c;

void main()
{
    gl_Position = vec4(vPos.x, vPos.y, vPos.z, 1.0);
    c = vec2(vPos.x*zoom, vPos.y*zoom) + offset;
}