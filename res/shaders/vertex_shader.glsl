#version 330 core

layout (location = 0) in vec3 vPos;

out vec2 pos;

void main()
{
    gl_Position = vec4(vPos.x, vPos.y, vPos.z, 1.0);
    pos = vec2(vPos.x, vPos.y);
}