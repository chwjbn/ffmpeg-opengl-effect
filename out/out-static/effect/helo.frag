#version 330 core

uniform sampler2D iTexture0;

in float ctxPlayTime;
in vec3 ctxColor;
in vec2 ctxTexCoord;

out vec4 color;

void main()
{
     color = vec4(1.0f, 0.5f, 0.2f, 1.0f);
}
