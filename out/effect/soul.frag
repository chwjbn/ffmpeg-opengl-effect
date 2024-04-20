#version 330 core

uniform sampler2D iTexture0;

in float ctxPlayTime;
in vec3 ctxColor;
in vec2 ctxTexCoord;

out vec4 color;

void main()
{
    float duration =0.9;
    float maxAlpha = 0.1;
    float maxScale = 1.5;

    float progress = mod(ctxPlayTime, duration) / duration;
    float alpha = maxAlpha * (1.0 - progress);
    float scale = 1.0 + (maxScale - 1.0) * progress;

    float weakX = 0.5 + (ctxTexCoord.x - 0.5) / scale;
    float weakY = 0.5 + (ctxTexCoord.y - 0.5) / scale;

    vec2 weakTextureCoords = vec2(weakX, weakY);
    vec4 weakMask = texture(iTexture0, weakTextureCoords);

    vec4 mask = texture(iTexture0, ctxTexCoord);

    color = mask * (1.0 - alpha) + weakMask * alpha;
}
