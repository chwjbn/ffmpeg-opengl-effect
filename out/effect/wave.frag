#version 330 core

uniform sampler2D iTexture0;

in float ctxPlayTime;
in vec3 ctxColor;
in vec2 ctxTexCoord;

out vec4 color;


void main()
{
    // 波浪的幅度
    float waveAmplitude = 0.02;
    
    // 波浪的频率
    float waveFrequency = 10.0;
    
    // 计算新的纹理坐标，使视频产生波浪效果
    vec2 texCoord = ctxTexCoord;
    texCoord.x += cos(texCoord.y * waveFrequency + ctxPlayTime) * waveAmplitude;
    texCoord.y += sin(texCoord.x * waveFrequency + ctxPlayTime) * waveAmplitude;
    
    // 获取纹理颜色
    color = texture(iTexture0, texCoord);
}
