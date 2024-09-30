#version 460

layout (set = 0, binding = 0) uniform sampler2D HBAO;

layout(location = 0) out float out_ao;

layout (location = 0) in vec2 inUV;

layout(push_constant) uniform round_setting {
    int index;
}ROUND_SETTING;


float kawase(sampler2D tex, vec2 uv, vec2 texelSize, int pixelOffset)
{
    float o = 0.0;
    o += texture(tex, uv + vec2(pixelOffset +0.5, pixelOffset +0.5) * texelSize).r; 
    o += texture(tex, uv + vec2(-pixelOffset -0.5, pixelOffset +0.5) * texelSize).r; 
    o += texture(tex, uv + vec2(-pixelOffset -0.5, -pixelOffset -0.5) * texelSize).r; 
    o += texture(tex, uv + vec2(pixelOffset +0.5, -pixelOffset -0.5) * texelSize).r; 
    return o * 0.25;
}

void main() 
{	
    vec2 size = vec2(1) / textureSize(HBAO, 0);

    out_ao = kawase(HBAO, inUV, size, ROUND_SETTING.index);
}