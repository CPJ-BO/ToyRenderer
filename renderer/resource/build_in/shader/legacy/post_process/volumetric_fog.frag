#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive : enable

#include "../include/common.glsl"
#include "../include/common_frag.glsl"

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 color;

layout(push_constant) uniform enable_setting {
    bool enable;
}ENABLE_SETTING;

void main() 
{
    ivec2 pixPos        = ivec2(fetchScreenPixPos(inUV));
    vec4 fogColor       = ENABLE_SETTING.enable ? imageLoad(VOLUMETRIC_FOG, pixPos) : vec4(0.0f, 0.0f, 0.0f, 1.0f);
    vec4 screenColor    = texture(COLOR_1_SAMPLER, inUV);

    float weight = fogColor.w;
    if(fetchDepth(inUV) == 1.0)    weight = 1.0;   //天光不衰减

    color               = vec4(fogColor.xyz + screenColor.xyz * weight, 1.0f);   //混合，输出，屏幕光照要乘以穿透率混合 
    //color             = vec4(fogColor.xyz, 1.0f);
}
