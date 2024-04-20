#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;
layout (location = 2) in vec2 texCoord;

uniform float iPlayTime;

out float ctxPlayTime;
out vec3 ctxColor;
out vec2 ctxTexCoord;

const float PI = 3.1415926;

void main()
{

    ctxPlayTime=iPlayTime;
    ctxColor = color;
    ctxTexCoord = texCoord;

    gl_Position = vec4(position, 1.0);
}
